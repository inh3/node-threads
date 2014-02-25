#ifndef _THREAD_ISOLATE_H_
#define _THREAD_ISOLATE_H_

// node
#include <node.h>
#include <node_version.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

// C++
#include <string>
using namespace std;

// custom
#include "file_info.h"

class ThreadIsolate
{
    public:

        // this should only be called once at module init
        static void     Initialize(const char* dirString);

        // global context (per thread)
        static void     InitializeGlobalContext();

        // copies the global context to module context
        static void     CloneGlobalContext(
                            Handle<Object> sourceObject,
                            Handle<Object> cloneObject);

        static void     CreateModuleContext(
                            Handle<Object> contextObject,
                            const FileInfo* fileInfo);

        static void     UpdateContextFileGlobals(
                            Handle<Object> contextObject,
                            const FileInfo* fileInfo);

    private:

        static bool         _IsInitialized;
        
        static FileInfo     _NativeModuleSupport;
        static FileInfo     _UtilFile;

        static string       _ModuleDir;
};

#endif /* _THREAD_ISOLATE_H_ */