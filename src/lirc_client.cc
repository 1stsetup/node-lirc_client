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
static Persistent<String> rawdata_symbol;
static Persistent<String> closed_symbol;
static Persistent<String> isConnected_symbol;
static Persistent<String> mode_symbol;

static int lircd_fd = -1;
static int lircd_conn_count = 0;
static Local<String> gProgramName;
static Handle<Boolean> gVerbose;

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
      rawdata_symbol = NODE_PSYMBOL("rawdata");
      data_symbol = NODE_PSYMBOL("data");
      closed_symbol = NODE_PSYMBOL("closed");

      isConnected_symbol = NODE_PSYMBOL("isConnected");
      mode_symbol = NODE_PSYMBOL("mode");

      NODE_SET_PROTOTYPE_METHOD(Lirc_client_constructor, "close", Close);
      NODE_SET_PROTOTYPE_METHOD(Lirc_client_constructor, "connect", Connect);

      Lirc_client_constructor->PrototypeTemplate()->SetAccessor(isConnected_symbol, IsConnectedGetter);
      Lirc_client_constructor->PrototypeTemplate()->SetAccessor(mode_symbol, ModeGetter, ModeSetter);

      target->Set(name, Lirc_client_constructor->GetFunction());
    }

    void init(Local<String> programname, Handle<Boolean> verbose, Local<Array> configfiles) {

	closed = true;
	start_r_poll = true;
	read_watcher_ = NULL;
	lirc_config_ = NULL;

printf("1 init\n");
	if (lircd_fd == -1) {
		char * writable = string2char(programname);
		lircd_fd = lirc_init(writable, verbose->Value() == true ? 1 : 0);
		delete[] writable;
	}
	
printf("2 init\n");
	if (lircd_fd < 0) {
		ThrowException(Exception::Error(String::New("Error on lirc_init.")));
		return;
	}

printf("3 init\n");
	lircd_conn_count++;

printf("4 init\n");
	if (configfiles->Length() > 0) {
		// Process each config file..
		int length = configfiles->Length();
		for(int i = 0; i < length; i++) {
			Local<Value> configfile = configfiles->Get(i);
			char * writable = string2char(configfile->ToString());
			if (lirc_readconfig(writable, &lirc_config_, NULL) != 0) {
				ThrowException(Exception::Error(String::Concat(String::New("Error on lirc_readconfig for file:"),configfile->ToString())));
				delete[] writable;
				return;
			}
			delete[] writable;
		}
	}
	else {
		if (lirc_readconfig(NULL, &lirc_config_, NULL) != 0) {
			ThrowException(Exception::Error(String::New("Error on lirc_readconfig.")));
			return;
		}
	}

printf("5 init\n");
	if (read_watcher_ == NULL) {
		read_watcher_ = new uv_poll_t;
		read_watcher_->data = this;
		// Setup input listener
		uv_poll_init(uv_default_loop(), read_watcher_, lircd_fd);
	}

printf("6 init\n");
	if (start_r_poll) {
		// Start input listener
		uv_poll_start(read_watcher_, UV_READABLE, io_event);
		start_r_poll = false;
	}

printf("7 init\n");
	closed = false;

    }

    static void on_handle_close (uv_handle_t *handle) {
	delete handle;
    }

    void close(bool doUnref) {

	if (doUnref) {
		Unref();
	}

	if (closed) return;

	uv_poll_stop(read_watcher_);
	uv_close((uv_handle_t *)read_watcher_, on_handle_close);

	read_watcher_ = NULL;
	start_r_poll = true;
	lirc_freeconfig(lirc_config_);

	lircd_conn_count--;
	if (lircd_conn_count == 0) {
		lirc_deinit();
		lircd_fd = -1;
	}
	closed = true;
    }

    void connect() {

	if (!closed) return;
printf("connect\n");
	init(gProgramName, gVerbose, v8::Array::New());
    }

  protected:
    static Handle<Value> New (const Arguments& args) {
	HandleScope scope;

	if (args.Length() > 3) {
		return ThrowException(Exception::TypeError(String::New("Only three arguments are allowed.")));
	}

	int prognameindex = -1;
	int verboseindex = -1;
	int configindex = -1;

	for(int i=0; i < args.Length(); i++) {
		if (args[i]->IsArray()) {
			if (configindex != -1) {
				return ThrowException(Exception::TypeError(String::New("Only one Array argument is allowed (config files).")));
			}
			configindex = i;
		}
		else if ((args[i]->IsString()) && (lircd_fd == -1)) {
			if (prognameindex != -1) {
				return ThrowException(Exception::TypeError(String::New("Only one string argument is allowed (progamname).")));
			}
			prognameindex = i;
		}
		else if (args[i]->IsBoolean()) {
			if (verboseindex != -1) {
				return ThrowException(Exception::TypeError(String::New("Only one boolean argument is allowed (verbose).")));
			}
			verboseindex = i;
		}
	}

	if ((lircd_fd == -1) && (prognameindex == -1)) {
		return ThrowException(Exception::Error(String::New("There is no lirc_client object with this.isConnected == true. So programname is required on new.")));
	}

	if ((configindex > -1) && (configindex < verboseindex)) {
		return ThrowException(Exception::TypeError(String::New("Order of arguments is wrong. verbose must be before config files.")));
	}

	if ((configindex > -1) && (configindex < prognameindex)) {
		return ThrowException(Exception::TypeError(String::New("Order of arguments is wrong. program name must be before config files.")));
	}

	if ((verboseindex > -1) && (verboseindex < prognameindex)) {
		return ThrowException(Exception::TypeError(String::New("Order of arguments is wrong. program name must be before verbose.")));
	}

	Lirc_client *lc = NULL;

	if (prognameindex > -1) {
		gProgramName = args[prognameindex]->ToString();
	}
	if ((verboseindex > -1) && (lircd_fd == -1)) {
		gVerbose = args[verboseindex]->ToBoolean();
	}

	lc = new Lirc_client(prognameindex > -1 ? args[prognameindex]->ToString() : String::New(""), verboseindex > -1 ? args[verboseindex]->ToBoolean() : Boolean::New(false), configindex > -1 ? Local<Array>::Cast(args[configindex]) : v8::Array::New());

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

      lc->close(false);

      return Undefined();
    }

    static Handle<Value> Connect (const Arguments& args) {
      Lirc_client *lc = ObjectWrap::Unwrap<Lirc_client>(args.This());
      HandleScope scope;

      lc->connect();

      return Undefined();
    }

    Lirc_client(Local<String> programname, Handle<Boolean> verbose, Local<Array> configfiles) : ObjectWrap() {
	this->init(programname, verbose, configfiles);
    }

    ~Lirc_client() {
	Emit.Dispose();
	Emit.Clear();
	close(true);
    }

    static Handle<Value> IsConnectedGetter (Local<String> property,
                                            const AccessorInfo& info) {
	Lirc_client *lc = ObjectWrap::Unwrap<Lirc_client>(info.This());
	assert(lc);
	assert(property == isConnected_symbol);

	HandleScope scope;

	return scope.Close(Boolean::New(!lc->closed));
    }

    static Handle<Value> ModeGetter (Local<String> property,
                                            const AccessorInfo& info) {
	Lirc_client *lc = ObjectWrap::Unwrap<Lirc_client>(info.This());
	assert(lc);
	assert(property == mode_symbol);

	HandleScope scope;

	const char *mode_ = lirc_getmode(lc->lirc_config_);

	if (mode_ == NULL) {
		return Undefined();
	}
	else {
		return scope.Close(String::New(mode_, strlen(mode_)));
	}
    }

    static void ModeSetter (Local<String> property, Local<Value> value,
                                            const AccessorInfo& info) {
	Lirc_client *lc = ObjectWrap::Unwrap<Lirc_client>(info.This());
	assert(lc);
	assert(property == mode_symbol);

	if (!value->IsString()) {
		ThrowException(Exception::TypeError(String::New("Mode should be a string value")));
	}

	char * writable = string2char(value->ToString());
	lirc_setmode(lc->lirc_config_, writable);
	delete[] writable;
    }

    private:
	bool start_r_poll;
	uv_poll_t* read_watcher_;
	struct lirc_config *lirc_config_;
	bool closed;

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
printf("code1: %s\n", code);
					Handle<Value> emit_argv[2] = {
						rawdata_symbol,
						String::New(code, strlen(code))
					};
					TryCatch try_catch;
					tmpClient->Emit->Call(tmpClient->handle_, 2, emit_argv);
					if (try_catch.HasCaught())
						FatalException(try_catch);

					while (((ret=lirc_code2char(tmpClient->lirc_config_,code,&c)) == 0) && (c != NULL)) {
						Handle<Value> emit_argv[2] = {
							data_symbol,
							String::New(c, strlen(c))
						};
						TryCatch try_catch;
						tmpClient->Emit->Call(tmpClient->handle_, 2, emit_argv);
						if (try_catch.HasCaught())
							FatalException(try_catch);
printf("code2: %s\n", code);
					}

					free(code);
				}
			}
			else {
				// Connection lircd got closed. Emit event.
				tmpClient->close(false);
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
