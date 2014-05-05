#include <lirc/lirc_client.h>

#include <stdlib.h>
#include <string.h>
#include <string>
#include <node.h>
#include <v8.h>

using namespace std;
using namespace v8;
using namespace node;

// https://github.com/mscdex/node-ncurses/blob/master/src/binding.cc
// Was used as an example for this source

static Persistent<FunctionTemplate> Lirc_client_constructor;
static Persistent<String> emit_symbol;
static Persistent<String> data_symbol;
static Persistent<String> closed_symbol;

char *string2char(const Local<String> avalue) {

	v8::String::Utf8Value utf8_value(avalue);

	std::string str = std::string(*utf8_value);
	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0'; // don't forget the terminating 0

	return writable;
}

class Lirc_client : public ObjectWrap {
  public:
    Persistent<Function> Emit;
    static void Initialize (Handle<Object> target) {
      HandleScope scope;

      Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
      Local<String> name = String::NewSymbol("client");

      Lirc_client_constructor = Persistent<FunctionTemplate>::New(tpl);
      Lirc_client_constructor->InstanceTemplate()->SetInternalFieldCount(1);
      Lirc_client_constructor->SetClassName(name);

      emit_symbol = NODE_PSYMBOL("emit");
      data_symbol = NODE_PSYMBOL("data");
      closed_symbol = NODE_PSYMBOL("closed");

      NODE_SET_PROTOTYPE_METHOD(Lirc_client_constructor, "close", Close);

      target->Set(name, Lirc_client_constructor->GetFunction());
    }

    void init(Local<String> programname, Handle<Boolean> verbose = v8::Boolean::New(false), Local<Array> configfiles = v8::Array::New()) {

	lircd_fd = -1;
	start_r_poll = true;
	read_watcher_ = NULL;
	lirc_config_ = NULL;

	char * writable = string2char(programname);
	lircd_fd = lirc_init(writable, verbose->Value() == true ? 1 : 0);
	delete[] writable;
	
	if (lircd_fd < 0) {
		ThrowException(Exception::Error(String::New("Error on lirc_init.")));
		return;
	}

	if (lirc_readconfig(NULL, &lirc_config_, NULL) != 0) {
		ThrowException(Exception::Error(String::New("Error on lirc_readconfig.")));
		return;
	}

	if (configfiles->Length() > 0) {
		// Process each config file..
	}

	if (read_watcher_ == NULL) {
		read_watcher_ = new uv_poll_t;
		read_watcher_->data = this;
		// Setup input listener
		uv_poll_init(uv_default_loop(), read_watcher_, lircd_fd);
	}

	if (start_r_poll) {
		// Start input listener
		uv_poll_start(read_watcher_, UV_READABLE, io_event);
		start_r_poll = false;
	}

    }

    static void on_handle_close (uv_handle_t *handle) {
	delete handle;
    }

    void close() {
	uv_poll_stop(read_watcher_);
	uv_close((uv_handle_t *)read_watcher_, on_handle_close);
	Unref();
	read_watcher_ = NULL;
	lircd_fd = -1;
	start_r_poll = true;
	lirc_freeconfig(lirc_config_);
	lirc_deinit();
    }

  protected:
    static Handle<Value> New (const Arguments& args) {
	HandleScope scope;

	if (args.Length() < 1) {
		return ThrowException(Exception::Error(String::New("Not enough arguments. Program name is minimally required.")));
	}

	if (!args[0]->IsString()) {
		return ThrowException(Exception::TypeError(String::New("First argument 'Programname' must be a string.")));
	}

	if ((args.Length() > 1) && (!args[1]->IsBoolean())) {
		return ThrowException(Exception::TypeError(String::New("Seconds argument 'verbode' must be a boolean.")));
	}

	if ((args.Length() > 2) && (!args[2]->IsArray())) {
		return ThrowException(Exception::TypeError(String::New("Third argument 'conigurationfiles' must be an array of strings.")));
	}

	Lirc_client *lc = NULL;
	if (args.Length() == 1) {
		lc = new Lirc_client(args[0]->ToString());
	}
	else if (args.Length() == 2) {
		lc = new Lirc_client(args[0]->ToString(), args[1]->ToBoolean());
	}
	else if (args.Length() == 3) {
		lc = new Lirc_client(args[0]->ToString(), args[1]->ToBoolean(), Local<Array>::Cast(args[2]));
	}

	if (lc != NULL) {
		lc->Wrap(args.This());
		lc->Ref();

		lc->Emit = Persistent<Function>::New(
			    Local<Function>::Cast(lc->handle_->Get(emit_symbol))
			  );
	}
	else {
		return ThrowException(Exception::Error(String::New("could not create new lirc_client object.")));
	}

	return args.This();
    }

    static Handle<Value> Close (const Arguments& args) {
      Lirc_client *lc = ObjectWrap::Unwrap<Lirc_client>(args.This());
      HandleScope scope;

      lc->close();

      return Undefined();
    }

    Lirc_client(Local<String> programname) : ObjectWrap() {
	this->init(programname);
    }

    Lirc_client(Local<String> programname, Local<Boolean> verbose) : ObjectWrap() {
	this->init(programname, verbose);
    }

    Lirc_client(Local<String> programname, Local<Boolean> verbose, Local<Array> configfiles) : ObjectWrap() {
	this->init(programname, verbose, configfiles);
    }

    ~Lirc_client() {
	Emit.Dispose();
	Emit.Clear();
    }

    private:
	int lircd_fd;
	bool start_r_poll;
	uv_poll_t* read_watcher_;
	struct lirc_config *lirc_config_;

	static void io_event (uv_poll_t* req, int status, int revents) {
		HandleScope scope;

		if (status < 0)
			return;

		if (revents & UV_READABLE) {
			char *code;
			char *c;
			int ret;

			Lirc_client *tmpClient = (Lirc_client *)req->data;

			int result = lirc_nextcode(&code);
			if (result == 0) {
				if (code != NULL) {
					while (((ret=lirc_code2char(tmpClient->lirc_config_,code,&c)) == 0) && (c != NULL)) {
						Handle<Value> emit_argv[2] = {
							data_symbol,
							String::New(c, strlen(c)+1)
						};
						TryCatch try_catch;
						tmpClient->Emit->Call(tmpClient->handle_, 2, emit_argv);
						if (try_catch.HasCaught())
							FatalException(try_catch);
					}

					free(code);
				}
			}
			else {
				// Connection lircd got closed. Emit event.
				tmpClient->close();
				Handle<Value> emit_argv[1] = {
					closed_symbol
				};
				TryCatch try_catch;
				tmpClient->Emit->Call(tmpClient->handle_, 1, emit_argv);
				if (try_catch.HasCaught())
					FatalException(try_catch);
			}
		}
	}


};

extern "C" {
  void init (Handle<Object> target) {
    HandleScope scope;
    Lirc_client::Initialize(target);
  }

  NODE_MODULE(lirc_client, init);
}
