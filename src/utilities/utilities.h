#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class Utilities
{
    public:

        static void             PrintObjectProperties(Handle<Object> objectHandle);

        static Handle<Value>    CompileScriptSource(Handle<String> scriptSource, const char* scriptResourceName = NULL);
};

#endif /* _UTILITIES_H_ */