#include <lirc/lirc_client.h>

#include <inttypes.h>
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

#define MAX_CONFIGS 20

static Persistent<String> emit_symbol;
static Persistent<String> data_symbol;
static Persistent<String> rawdata_symbol;
static Persistent<String> closed_symbol;
static Persistent<String> isConnected_symbol;
static Persistent<String> mode_symbol;
static Persistent<String> configFiles_symbol;
static Persistent<Function> global_cb;

static int lircd_fd = -1;
static Local<String> gProgramName;
static Handle<Boolean> gVerbose;

static uv_poll_t *read_watcher_ = NULL;

struct Tlirc_config {
	struct lirc_config *lirc_config_;
};

static Tlirc_config *my_lirc_config = new Tlirc_config[MAX_CONFIGS];
static bool closed = true;
static Persistent<Array> configFiles_;


char *string2char(const Local<String> avalue) {

	v8::String::Utf8Value utf8_value(avalue);

	std::string str = std::string(*utf8_value);
	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0'; // don't forget the terminating 0

	return writable;
}

static void io_event (uv_poll_t* req, int status, int revents);
void addConfig(Local<String> name);
void addConfig(Handle<Array> name);
void connect(Handle<String> programname, Handle<Boolean> verbose, Handle<Array> configfiles, Handle<Function> cb);

void connect(Handle<String> programname, Handle<Boolean> verbose, Handle<String> configfiles, Handle<Function> cb) {
	Local<Array> tmpArray = Array::New(1);
	tmpArray->Set(Number::New(0), configfiles);
	connect(programname, verbose, tmpArray, cb);
}

void connect(Handle<String> programname, Handle<Boolean> verbose, Handle<Array> configfiles, Handle<Function> cb) {

printf("0a init\n"); fflush(NULL);
	if (!closed) return;

printf("0b init\n"); fflush(NULL);
	closed = true;

printf("0c init\n"); fflush(NULL);
	gProgramName = String::Concat(programname, String::New(""));
printf("0d init\n"); fflush(NULL);
	gVerbose = verbose;

printf("0e init\n"); fflush(NULL);
	for (int i=0; i < MAX_CONFIGS; i++) {
		my_lirc_config[i].lirc_config_ = NULL;
	}

printf("1 init\n"); fflush(NULL);
	if (lircd_fd == -1) {
		char * writable = string2char(gProgramName);
		lircd_fd = lirc_init(writable, verbose->Value() == true ? 1 : 0);
		delete[] writable;
		printf("1a lircd_fd:%d\n",lircd_fd);
	}
	
printf("2 init\n");
	if (lircd_fd < 0) {
		ThrowException(Exception::Error(String::New("Error on lirc_init.")));
		return;
	}

printf("4 init\n");
	configFiles_ = Persistent<Array>::New( Array::New() );
	addConfig(configfiles);

printf("5 init\n");
	if (read_watcher_ == NULL) {
printf("5a init\n");
		read_watcher_ = new uv_poll_t;
		read_watcher_->data = my_lirc_config;
		// Setup input listener
		uv_poll_init(uv_default_loop(), read_watcher_, lircd_fd);
printf("6b init\n");
		// Start input listener
		uv_poll_start(read_watcher_, UV_READABLE, io_event);
	}

	global_cb = Persistent<Function>::New(cb);

printf("7 init\n");
	closed = false;

}

static void on_handle_close (uv_handle_t *handle) {
printf("on_handle_close\n");
	delete handle;
}

void close() {

	if (closed) return;

	for (int i=0; i < MAX_CONFIGS; i++) {
		if (my_lirc_config[i].lirc_config_ != NULL) {
			lirc_freeconfig(my_lirc_config[i].lirc_config_);
		}
		my_lirc_config[i].lirc_config_ = NULL;
	}

	uv_poll_stop(read_watcher_);
	uv_close((uv_handle_t *)read_watcher_, on_handle_close);
printf("uv_close\n");

	read_watcher_ = NULL;
printf("lirc_deinit\n");
	lirc_deinit();
	lircd_fd = -1;

	closed = true;
}

void addConfig(Local<String> name) {

	int i = 0;
	while ((i < MAX_CONFIGS) && (my_lirc_config[i].lirc_config_ != NULL)) {
		i++;
	}

	if (i < MAX_CONFIGS) {
		char * writable = NULL;
		if (name->Length() > 0) {
			writable = string2char(name);
		}

		if (lirc_readconfig(writable, &(my_lirc_config[i].lirc_config_), NULL) != 0) {
			ThrowException(Exception::Error(String::Concat(String::New("Error on lirc_readconfig for file:"),name)));
			delete[] writable;
			return;
		}
	
		delete[] writable;

		uint32_t oldLength = configFiles_->Length();

		configFiles_->Set(String::New("length"), Number::New(oldLength+1));
		configFiles_->Set(Number::New(oldLength), name);
	}
	else {
		ThrowException(Exception::Error(String::New("Config buffer is full.")));
	}
}

void addConfig(Handle<Array> names) {

	int length = names->Length();
	for(int i = 0; i < length; i++) {
		if (!names->Get(i)->IsString()) {
			ThrowException(Exception::Error(String::Concat(String::New("Array element is not a String:"),names->Get(i)->ToString())));
			return;
		}
		Local<Value> configfile = names->Get(i);
		addConfig(configfile->ToString());
	}
}

void clearConfig() {

	configFiles_->Set(String::New("length"), Number::New(0));

	for (int i=0; i < MAX_CONFIGS; i++) {
		if (my_lirc_config[i].lirc_config_ != NULL) {
			lirc_freeconfig(my_lirc_config[i].lirc_config_);
			my_lirc_config[i].lirc_config_ = NULL;
		}
	}
}


static Handle<Value> Connect (const Arguments& args) {
	HandleScope scope;

	if (!closed) {
		return Undefined();
	}

	if (args.Length() > 4) {
		return ThrowException(Exception::TypeError(String::New("Only four arguments are allowed.")));
	}

	int prognameindex = -1;
	int verboseindex = -1;
	int configindex = -1;
	int cbindex = -1;

	for(int i=0; i < args.Length(); i++) {
		if (args[i]->IsString()) {
			if ((prognameindex != -1) && (configindex != -1)) {
				return ThrowException(Exception::TypeError(String::New("Only two String argument are allowed (Program name and Config files).")));
			}
			if (prognameindex == -1) {
				prognameindex = i;
			}
			else {
				configindex = i;
			}
		}
		else if (args[i]->IsBoolean()) {
			if (verboseindex != -1) {
				return ThrowException(Exception::TypeError(String::New("Only one boolean argument is allowed (verbose).")));
			}
			verboseindex = i;
		}
		else if (args[i]->IsFunction()) {
			if (cbindex != -1) {
				return ThrowException(Exception::TypeError(String::New("Only one Callback Function argument is allowed (cb).")));
			}
			cbindex = i;
		}
	}

	if (prognameindex == -1) {
		return ThrowException(Exception::Error(String::New("Programname is required.")));
	}

	if (cbindex == -1) {
		return ThrowException(Exception::Error(String::New("Callback function is required.")));
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

	connect( args[prognameindex]->ToString(), verboseindex > -1 ? args[verboseindex]->ToBoolean() : Boolean::New(false), configindex > -1 ? args[configindex]->ToString() : String::New(""), Local<Function>::Cast(args[cbindex]));

	return Undefined();
}

static Handle<Value> ReConnect (const Arguments& args) {
	HandleScope scope;

	if (!closed) {
		return Undefined();
	}

	int length = configFiles_->Length();
	Local<Array> tmpArray = Array::New(length);
	for (int i = 0; i < length; i++) {
		tmpArray->Set(Number::New(i), configFiles_->Get(i));
	}
printf("hiero1\n");
	connect(gProgramName, gVerbose, tmpArray, global_cb);

	return Undefined();
}

static Handle<Value> Close (const Arguments& args) {
      HandleScope scope;

      close();

      return Undefined();
}

static Handle<Value> AddConfig (const Arguments& args) {
	HandleScope scope;

	if (args.Length() != 1) {
		return ThrowException(Exception::TypeError(String::New("Only one Array or String argument is allowed.")));
	}

	if (args[0]->IsArray()) {
		addConfig(Local<Array>::Cast(args[0]));
	}
	else if (args[0]->IsString()) {
		addConfig(args[0]->ToString());
	}
	else {
		return ThrowException(Exception::TypeError(String::New("Only an Array or a String argument is allowed.")));
	}

	return Undefined();
}

static Handle<Value> ClearConfig (const Arguments& args) {
	HandleScope scope;

	clearConfig();

	return Undefined();
}

static Handle<Value> IsConnectedGetter (Local<String> property,
                                            const AccessorInfo& info) {
	assert(property == isConnected_symbol);

	HandleScope scope;

	return scope.Close(Boolean::New(!closed));
}

static Handle<Value> ModeGetter (Local<String> property,
                                            const AccessorInfo& info) {
	assert(property == mode_symbol);

	HandleScope scope;

	const char *mode_ = NULL;
	if (my_lirc_config[0].lirc_config_ != NULL) {
		lirc_getmode(my_lirc_config[0].lirc_config_);
	}

	if (mode_ == NULL) {
		return Undefined();
	}
	else {
		return scope.Close(String::New(mode_, strlen(mode_)));
	}
}

static void ModeSetter (Local<String> property, Local<Value> value,
                                            const AccessorInfo& info) {
	assert(property == mode_symbol);

	HandleScope scope;

	if (!value->IsString()) {
		ThrowException(Exception::TypeError(String::New("Mode should be a string value")));
		return;
	}

	char * writable = string2char(value->ToString());

	if (my_lirc_config[0].lirc_config_ != NULL) {
		lirc_setmode(my_lirc_config[0].lirc_config_, writable);
	}
	else {
		ThrowException(Exception::TypeError(String::New("Cannot set mode on empty config")));
	}
	delete[] writable;
}

static Handle<Value> ConfigFilesGetter (Local<String> property,
                                            const AccessorInfo& info) {
	assert(property == configFiles_symbol);

	HandleScope scope;

	return scope.Close(configFiles_);
}


static void io_event (uv_poll_t* req, int status, int revents) {
	HandleScope scope;

	if (status < 0)
		return;

	if (revents & UV_READABLE) {
		char *code;
		char *c;
		int ret;

		int result = lirc_nextcode(&code);
		if (result == 0) {
			if (code != NULL) {

				// Send rawdata event
				Handle<Value> emit_argv[2] = {
					rawdata_symbol,
					String::New(code, strlen(code))
				};
				TryCatch try_catch;
				global_cb->Call(Context::GetCurrent()->Global(), 2, emit_argv);
				if (try_catch.HasCaught())
					FatalException(try_catch);

				for (int i=0; i<MAX_CONFIGS; i++) {
					if (my_lirc_config[i].lirc_config_ != NULL) {
	printf("2. Trying config '%d'.\n", i);
						while (((ret=lirc_code2char(my_lirc_config[i].lirc_config_,code,&c)) == 0) && (c != NULL)) {
							// Send data event.
	printf("3. Trying config '%d'.\n", i);
							Handle<Value> emit_argv[2] = {
								data_symbol,
								String::New(c, strlen(c))
							};
							TryCatch try_catch;
							global_cb->Call(Context::GetCurrent()->Global(), 2, emit_argv);
							if (try_catch.HasCaught())
								FatalException(try_catch);
						}
					}
				}

				free(code);
			}
		}
		else {
			// Connection lircd got closed. Emit event.
			// Send closed event
			close();
			Handle<Value> emit_argv[1] = {
				closed_symbol
			};
			TryCatch try_catch;
			global_cb->Call(Context::GetCurrent()->Global(), 1, emit_argv);
			if (try_catch.HasCaught())
				FatalException(try_catch);
		}
	}
}


extern "C" {
  void init (Handle<Object> target) {
	HandleScope scope;

	read_watcher_ = NULL;

	emit_symbol = NODE_PSYMBOL("emit");
	rawdata_symbol = NODE_PSYMBOL("rawdata");
	data_symbol = NODE_PSYMBOL("data");
	closed_symbol = NODE_PSYMBOL("closed");

	isConnected_symbol = NODE_PSYMBOL("isConnected");
	mode_symbol = NODE_PSYMBOL("mode");
	configFiles_symbol = NODE_PSYMBOL("configFiles");

	target->Set(String::NewSymbol("close"), FunctionTemplate::New(Close)->GetFunction());
	target->Set(String::NewSymbol("connect"), FunctionTemplate::New(Connect)->GetFunction());
	target->Set(String::NewSymbol("reConnect"), FunctionTemplate::New(ReConnect)->GetFunction());
	target->Set(String::NewSymbol("addConfig"), FunctionTemplate::New(AddConfig)->GetFunction());
	target->Set(String::NewSymbol("clearConfig"), FunctionTemplate::New(ClearConfig)->GetFunction());

	target->SetAccessor(isConnected_symbol, IsConnectedGetter);
	target->SetAccessor(mode_symbol, ModeGetter, ModeSetter);
	target->SetAccessor(configFiles_symbol, ConfigFilesGetter);
  }

  NODE_MODULE(lirc_client, init);
}
