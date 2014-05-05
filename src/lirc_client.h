#ifndef LIRC_CLIENT_H
#define LIRC_CLIENT_H

#include <node.h>

class lirc_client : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit lirc_client(double value = 0);
  ~lirc_client();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
  double value_;
};

#endif
