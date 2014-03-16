#ifndef _REQUIRE_H_
#define _REQUIRE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;
using namespace node;

// C++
#include <string>
using namespace std;

#include "nan.h"

// custom
#include "file_info.h"

class Require
{
    public:

        // should be called once per isolate
        static void InitializePerIsolate();
        
        static NAN_METHOD(RequireMethod);

        static string ModuleDir;

    private:

        static void LoadNativeModule(Handle<String> moduleName, FileInfo* nativeFileInfo);

        // ensure default constructor can't get called
        Require();

        // ensure copy constructor methods can't be called
        Require(Require const&);
        void operator=(Require const&);
};

#endif /* _REQUIRE_H_ */