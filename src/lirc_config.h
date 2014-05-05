#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <node.h>

class jsLirc_Config : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit jsLirc_Config(double value = 0);
  ~jsLirc_Config();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
  double value_;
};

#endif
