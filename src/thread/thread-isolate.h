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

        // global context (per thread)
        static void     InitializeGlobalContext(bool isWorker = false);

        // copies the global context to module context
        static void     CloneGlobalContext(
                            Handle<Object> sourceObject,
                            Handle<Object> cloneObject);

        static void     CreateWorkerContext(Handle<Object> contextObject);

        static void     CreateModuleContext(
                            Handle<Object> contextObject,
                            const FileInfo* fileInfo);

        static void     UpdateContextFileGlobals(
                            Handle<Object> contextObject,
                            const FileInfo* fileInfo);

    private:

        static bool         _IsInitialized;
        static string       _ProcessDir;
};

#endif /* _THREAD_ISOLATE_H_ */