#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

class Environment
{
    public:

        static Persistent<Value>        NumCPUs;
        static Persistent<Object>       Path;
        static Persistent<Function>     EventEmitter;
        static Persistent<Function>     CalleeByStackTrace;
        static Persistent<Function>     Guid;

        static void Initialize(const char* moduleDir, const char* processDir);

    private:

        static void GuidInitialize(const char* moduleDir);
        static void StackTraceInitialize(const char* moduleDir);
};

#endif /* _ENVIRONMENT_H_ */