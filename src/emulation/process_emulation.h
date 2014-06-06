#ifndef _PROCESS_EMULATION_H_
#define _PROCESS_EMULATION_H_

// node
//using namespace node;
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

        static NAN_METHOD(StdOutWrite);
        static NAN_METHOD(StdErrWrite);

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

#endif /* _PROCESS_EMULATION_H_ */
