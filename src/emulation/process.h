#ifndef _PROCESS_H
#define _PROCESS_H

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

// C++
#include <string>
using namespace std;

class Process
{
    public:
        
        // should only be called once from main thread
        static void Initialize();

        // should be called once per isolate
        static Handle<Object> GetIsolateProcess();

    private:

        // property accessors
        static NAN_GETTER(GetArch);
        static NAN_GETTER(GetEnv);
        static NAN_GETTER(GetPlatform);

        static string Arch;
        static string Env;
        static string Platform;

        // ensure default constructor can't get called
        Process();

        // ensure copy constructor methods can't be called
        Process(Process const&);
        void operator=(Process const&);
};

#endif /* _PROCESS_H */