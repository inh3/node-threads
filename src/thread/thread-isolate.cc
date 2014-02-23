#define _THREAD_ISOLATE_CC_

#include "thread-isolate.h"

// node
#include <node_version.h>

// custom
#include "thread.h"
#include "utilities.h"

bool ThreadIsolate::_IsInitialized = false;
FileInfo ThreadIsolate::_UtilFile;

void ThreadIsolate::Initialize()
{
    // make sure to only initialize once
    if(_IsInitialized == false)
    {
        _IsInitialized = true;

        _UtilFile.LoadFile("./src/js/util.js");
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

    // global namespace object
    globalContext->Set(String::NewSymbol("global"), Object::New());

    // require(...)

    // get handle to nRequire function
    //Local<FunctionTemplate> functionTemplate = FunctionTemplate::New(Require::RequireFunction);
    //Local<Function> requireFunction = functionTemplate->GetFunction();
    //requireFunction->SetName(String::NewSymbol("require"));

    // attach function to context
    //globalContext->Set(String::NewSymbol("require"), requireFunction);

    // console.log(...)

    // create console object
    Handle<Object> consoleObject = Object::New();

    // store reference to node util object
    Handle<Value> scriptResult = Utilities::CompileScriptSource(String::New(_UtilFile.FileContents()));
    NanAssignPersistent(Object, threadContext->node_util, scriptResult->ToObject());

    // get handle to log function
    Local<FunctionTemplate> logTemplate = FunctionTemplate::New(ThreadIsolate::ConsoleLog);
    Local<Function> logFunction = logTemplate->GetFunction();
    logFunction->SetName(String::NewSymbol("log"));

    // attach log function to console object
    consoleObject->Set(String::NewSymbol("log"), logFunction);

    // attach object to context
    globalContext->Set(String::NewSymbol("console"), consoleObject);
}

// node -----------------------------------------------------------------------

NAN_METHOD(ThreadIsolate::ConsoleLog)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

    // the handle for node's util module
    Local<Object> nodeUtil;

    // Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
    nodeUtil = Local<Object>::New(isolate, threadContext->node_util);
#else
    HandleScope handleScope;
    nodeUtil = Local<Object>::New(threadContext->node_util);
#endif

    Local<Value> inspectArgs[] = { args[0] };
    Local<Function> utilInspect = nodeUtil->Get(String::NewSymbol("inspect")).As<Function>();
    Handle<Value> inspectResult = utilInspect->Call(args.This(), 1, inspectArgs);

    String::Utf8Value logMessage(inspectResult->ToString());
    printf("%s\n", *logMessage);

    NanReturnUndefined();
}