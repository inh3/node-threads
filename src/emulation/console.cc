#define _CONSOLE_CC_

#include "console.h"

// C
#include <stdio.h>

// custom
#include "thread.h"
#include "utilities.h"

Handle<Object> Console::GetIsolateConsole()
{
    // create console object
    Handle<Object> consoleObject = Object::New();

    // get handle to log function
    Local<FunctionTemplate> logTemplate = FunctionTemplate::New(Console::Log);
    Local<Function> logFunction = logTemplate->GetFunction();
    logFunction->SetName(String::NewSymbol("log"));

    // attach log function to console object
    consoleObject->Set(String::NewSymbol("log"), logFunction);

    return consoleObject;
}

// node -----------------------------------------------------------------------

NAN_METHOD(Console::Log)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

    // Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    NativeMap::const_iterator nativeModuleItr = threadContext->native_modules->find("util");
    Local<Object> utilObject = NanObjectWrapHandle(nativeModuleItr->second);

    Local<Value> inspectArgs[] = { args[0] };
    Local<Function> utilInspect = utilObject->Get(String::NewSymbol("inspect")).As<Function>();
    Handle<Value> inspectResult = utilInspect->Call(args.This(), 1, inspectArgs);

    String::Utf8Value logMessage(inspectResult->ToString());
    printf("%s\n", *logMessage);

    NanReturnUndefined();
}