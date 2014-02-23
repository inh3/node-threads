#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

class Utilities
{
    public:

        static const char*      ToCString(const String::Utf8Value& value);

        static void             PrintObjectProperties(Handle<Object> objectHandle);

        static Handle<Value>    CompileScriptSource(Handle<String> scriptSource, const char* scriptResourceName = 0);
};

#endif /* _UTILITIES_H_ */