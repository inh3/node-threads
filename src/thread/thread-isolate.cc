#define _THREAD_ISOLATE_CC_

#include "thread-isolate.h"

// custom
#include "thread.h"
#include "console.h"
#include "process_emulation.h"
#include "require.h"
#include "json.h"
#include "utilities.h"

bool ThreadIsolate::_IsInitialized = false;
FileInfo ThreadIsolate::_UtilFile;
string ThreadIsolate::_ModuleDir;

// this should only be called from main thread
void ThreadIsolate::Initialize(const char* dirString)
{
    // make sure to only initialize once
    if(_IsInitialized == false)
    {
        _IsInitialized = true;

        _ModuleDir.assign(dirString);
        _UtilFile.LoadFile("./src/js/util.js");

        Process::Initialize();
    }
}

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

    // initialize the json object
    JsonUtility::Initialize();

    // global namespace object
    globalContext->Set(String::NewSymbol("global"), Object::New());

    // process ----------------------------------------------------------------

    // attach object to context
    globalContext->Set(String::NewSymbol("process"), Process::GetIsolateProcess());

    // require(...) -----------------------------------------------------------

    // get handle to nRequire function
    Local<FunctionTemplate> functionTemplate = FunctionTemplate::New(Require::RequireMethod);
    Local<Function> requireFunction = functionTemplate->GetFunction();
    requireFunction->SetName(String::NewSymbol("require"));

    // attach function to context
    globalContext->Set(String::NewSymbol("require"), requireFunction);

    // console ----------------------------------------------------------------

    // attach object to context
    globalContext->Set(String::NewSymbol("console"), Console::GetIsolateConsole(_UtilFile));

    // /Utilities::PrintObjectProperties(globalContext);
}

void ThreadIsolate::CloneGlobalContext(Handle<Object> sourceObject, Handle<Object> cloneObject)
{
    NanScope();
    
    // copy global properties
    cloneObject->Set(String::NewSymbol("global"), sourceObject->Get(String::NewSymbol("global")));
    cloneObject->Set(String::NewSymbol("console"), sourceObject->Get(String::NewSymbol("console")));
    cloneObject->Set(String::NewSymbol("process"), sourceObject->Get(String::NewSymbol("process")));
    cloneObject->Set(String::NewSymbol("require"), sourceObject->Get(String::NewSymbol("require")));
}

void ThreadIsolate::CreateModuleContext(Handle<Object> contextObject, const FileInfo* fileInfo)
{
    NanScope();

    // create the module/exports within context
    Handle<Object> moduleObject = Object::New();
    moduleObject->Set(String::NewSymbol("exports"), Object::New());
    contextObject->Set(String::NewSymbol("module"), moduleObject);
    contextObject->Set(String::NewSymbol("exports"), moduleObject->Get(String::NewSymbol("exports"))->ToObject());

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
    contextObject->Set(String::NewSymbol("__dirname"), String::New(fileInfo->folderPath));
    contextObject->Set(String::NewSymbol("__filename"), String::New(fileInfo->fullPath));
}