#include <lirc/lirc_client.h>

#include <string>
#include <node.h>
#include "lirc_client.h"
#include <v8.h>


using namespace v8;

char *string2char(const Local<String> avalue) {

	v8::String::Utf8Value utf8_value(avalue);

	std::string str = std::string(*utf8_value);
	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0'; // don't forget the terminating 0

	return writable;
}

//int lirc_init(char *prog,int verbose);
Handle<Value> init(const Arguments& args) {
	HandleScope scope;

	if (args.Length() < 2) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}

	if (!args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("First argument needs to be a string")));
		return scope.Close(Undefined());
	}

	if (!args[1]->IsBoolean()) {
		ThrowException(Exception::TypeError(String::New("Second argument need to be a boolean value")));
		return scope.Close(Undefined());
	}

	int verbose = 0;
	if (args[1]->BooleanValue()) {
		verbose = 1;
	}

	char * writable = string2char(args[0]->ToString());

	//int lirc_init(char *prog,int verbose);
	int result = lirc_init(writable, verbose);
	Local<Number> num = Number::New(result);

	delete[] writable;
//  Local<String> num = String::Concat(String::Concat(args[0]->ToString(), String::New(":")), args[1]->ToString());

	return scope.Close(num);
}

//int lirc_readconfig(char *file,struct lirc_config **config,
//		    int (check)(char *s));
Handle<Value> readconfig(const Arguments& args) {
	HandleScope scope;

	if (args.Length() < 2) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}

	if (!args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("First argument needs to be a string")));
		return scope.Close(Undefined());
	}

	if (!args[1]->IsBoolean()) {
		ThrowException(Exception::TypeError(String::New("Second argument need to be a boolean value")));
		return scope.Close(Undefined());
	}

	int verbose = 0;
	if (args[1]->BooleanValue()) {
		verbose = 1;
	}

	char * writable = string2char(args[0]->ToString());

	//int lirc_init(char *prog,int verbose);
	int result = lirc_init(writable, verbose);
	Local<Number> num = Number::New(result);

	delete[] writable;
//  Local<String> num = String::Concat(String::Concat(args[0]->ToString(), String::New(":")), args[1]->ToString());

	return scope.Close(num);
}

void initialize(Handle<Object> exports) {
	exports->Set(String::NewSymbol("lirc_init"),
		FunctionTemplate::New(init)->GetFunction());
	exports->Set(String::NewSymbol("lirc_readconfig"),
		FunctionTemplate::New(readconfig)->GetFunction());
	
}

NODE_MODULE(lirc_client, initialize)
