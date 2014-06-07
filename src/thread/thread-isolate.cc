    #define _THREAD_ISOLATE_CC_

#include "thread-isolate.h"

// custom
#include "thread.h"
#include "process_emulation.h"
#include "require.h"
#include "json.h"
#include "utilities.h"

#include "nan-extra.h"

void ThreadIsolate::InitializeGlobalContext()
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);

    NanScope();

    // create a isolate context for the javascript in this thread
    Local<Context> isolateContext = NanNewContextHandle();
    NanAssignPersistent(
        threadContext->isolate_context,
        isolateContext);

    // enter thread specific context
    isolateContext->Enter();

    // get usable handle to the context's object
    Local<Object> globalContext = NanGetCurrentContext()->Global();

    // global namespace object
    globalContext->Set(NanNew<String>("global"), NanNew<Object>());

    // process ----------------------------------------------------------------

    // attach object to context
    globalContext->Set(NanNew<String>("process"), Process::GetIsolateProcess());

    // require(...) -----------------------------------------------------------

    // get handle to require function
    Local<FunctionTemplate> functionTemplate = NanNew<FunctionTemplate>(Require::RequireMethod);
    Local<Function> requireFunction = functionTemplate->GetFunction();
    requireFunction->SetName(NanNew<String>("require"));

    // attach function to context
    globalContext->Set(NanNew<String>("require"), requireFunction);

    // initialize require
    Require::InitializePerIsolate();

    // console ----------------------------------------------------------------

    // get the console module
    NativeMap::const_iterator consoleModuleItr =
            threadContext->native_modules->find("console");
    Local<Object> consoleObject = NanNew(NanObjectWrapHandle(consoleModuleItr->second));

    // attach object to context
    globalContext->Set(NanNew<String>("console"), consoleObject);

    // exit thread specific context
    isolateContext->Exit();
}

void ThreadIsolate::CloneGlobalContext(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
    NanScope();
    
    // copy global properties
    cloneObject->Set(NanNew<String>("global"), sourceObject->Get(NanNew<String>("global")));
    cloneObject->Set(NanNew<String>("console"), sourceObject->Get(NanNew<String>("console")));
    cloneObject->Set(NanNew<String>("process"), sourceObject->Get(NanNew<String>("process")));
    cloneObject->Set(NanNew<String>("require"), sourceObject->Get(NanNew<String>("require")));
}

void ThreadIsolate::CreateModuleContext(Handle<Object> contextObject, const FileInfo* fileInfo)
{
    NanScope();

    // create the module/exports within context
    Handle<Object> moduleObject = NanNew<Object>();
    moduleObject->Set(NanNew<String>("exports"), NanNew<Object>());
    contextObject->Set(NanNew<String>("module"), moduleObject);
    contextObject->Set(NanNew<String>("exports"), moduleObject->Get(NanNew<String>("exports"))->ToObject());

    // copy file properties
    if(fileInfo != NULL)
    {
        ThreadIsolate::UpdateContextFileGlobals(contextObject, fileInfo);
    }
}

void ThreadIsolate::UpdateContextFileGlobals(Handle<Object> contextObject, const FileInfo* fileInfo)
{
    NanScope();

    // set the file properites on the context
    contextObject->Set(NanNew<String>("__dirname"), NanNew<String>(fileInfo->folderPath));
    contextObject->Set(NanNew<String>("__filename"), NanNew<String>(fileInfo->fullPath));
}
