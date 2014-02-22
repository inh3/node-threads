#define _THREAD_ISOLATE_CC_

#include "thread-isolate.h"

// node
#include <node_version.h>

void ThreadIsolate::InitializeGlobalContext(Isolate* isolate, Handle<Object> globalContext)
{
// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
        HandleScope handleScope(isolate);
#else
        HandleScope handleScope;
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

    // setup console object
    //Handle<Object> consoleObject = Object::New();

    // get handle to log function
    //Local<FunctionTemplate> logTemplate = FunctionTemplate::New(ConsoleLog);
    //Local<Function> logFunction = logTemplate->GetFunction();
    //logFunction->SetName(String::NewSymbol("log"));

    // attach log function to console object
    //consoleObject->Set(String::NewSymbol("log"), logFunction);

    // attach object to context
    //globalContext->Set(String::NewSymbol("console"), consoleObject);
}