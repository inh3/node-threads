#define _THREAD_ISOLATE_CC_

#include "thread-isolate.h"

// custom
#include "thread.h"
#include "process_emulation.h"
#include "require.h"
#include "json.h"
#include "utilities.h"

void ThreadIsolate::InitializeGlobalContext()
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);

    // create a isolate context for the javascript in this thread
    NanAssignPersistent(Context, threadContext->isolate_context, Context::New(isolate));

    // enter thread specific context
    Context::Scope contextScope(threadContext->thread_isolate, threadContext->isolate_context);

    // get usable handle to the context's object
    Local<Object> globalContext = Local<Context>::New(isolate, threadContext->isolate_context)->Global();
#else
    HandleScope handleScope;

    // create a isolate context for the javascript in this thread
    NanAssignPersistent(Context, threadContext->isolate_context, Context::New());

    // enter thread specific context
    Context::Scope contextScope(threadContext->isolate_context);

    // get usable handle to the context's object
    Handle<Object> globalContext = threadContext->isolate_context->Global();
#endif

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

    // attempt to find the node thread instance by its name
    NativeMap::const_iterator consoleModuleItr =
            threadContext->native_modules->find("console");
#if (NODE_MODULE_VERSION > 0x000B)
    Local<Object> consoleObject = NanObjectWrapHandle(consoleModuleItr->second);
#else
    Local<Object> consoleObject = Local<Object>::New(
        NanObjectWrapHandle(consoleModuleItr->second));
#endif

    // attach object to context
    globalContext->Set(NanNew<String>("console"), consoleObject);
}

void ThreadIsolate::CloneGlobalContext(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
#if (NODE_MODULE_VERSION > 0x000B)
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope scope(isolate);
#else
        HandleScope scope;
#endif
    
    // copy global properties
    cloneObject->Set(NanNew<String>("global"), sourceObject->Get(NanNew<String>("global")));
    cloneObject->Set(NanNew<String>("console"), sourceObject->Get(NanNew<String>("console")));
    cloneObject->Set(NanNew<String>("process"), sourceObject->Get(NanNew<String>("process")));
    cloneObject->Set(NanNew<String>("require"), sourceObject->Get(NanNew<String>("require")));
}

void ThreadIsolate::CreateModuleContext(Handle<Object> contextObject, const FileInfo* fileInfo)
{
#if (NODE_MODULE_VERSION > 0x000B)
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope scope(isolate);
#else
        HandleScope scope;
#endif

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
#if (NODE_MODULE_VERSION > 0x000B)
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope scope(isolate);
#else
        HandleScope scope;
#endif

    // set the file properites on the context
    contextObject->Set(NanNew<String>("__dirname"), NanNew<String>(fileInfo->folderPath));
    contextObject->Set(NanNew<String>("__filename"), NanNew<String>(fileInfo->fullPath));
}
