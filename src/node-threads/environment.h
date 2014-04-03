#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// C++
#include <string>
using namespace std;

class Environment
{
    public:

        // overall module global references
        static Persistent<Object>      Exports;
        static Persistent<Object>      Module;

        // node global references
        static Persistent<Function>     EventEmitter;
        static Persistent<Object>       Path;
        static Persistent<Object>       Util;

        // custom global references
        static Persistent<Value>        NumCPUs;
        static Persistent<Function>     CalleeByStackTrace;
        static Persistent<Function>     Guid;

        static string                   ModuleDir;
        static string                   ProcessDir;

        static void Initialize(const char* moduleDir, const char* processDir);

    private:

        static void GuidInitialize(const char* moduleDir);
        static void NodeInitialize();
        static void StackTraceInitialize(const char* moduleDir);

        static bool                     _IsInitialized;
};

#endif /* _ENVIRONMENT_H_ */