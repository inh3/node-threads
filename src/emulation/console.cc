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

    NativeMap::const_iterator nativeModuleItr = threadContext->native_modules->find("util");

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
    Local<Object> utilObject = NanObjectWrapHandle(nativeModuleItr->second);
#else
    HandleScope handleScope;
    Local<Object> utilObject = Local<Object>::New(
        NanObjectWrapHandle(nativeModuleItr->second));
#endif

    Local<Value> inspectArgs[] = { args[0] };
    Local<Function> utilInspect = utilObject->Get(String::NewSymbol("inspect")).As<Function>();
    Handle<Value> inspectResult = utilInspect->Call(args.This(), 1, inspectArgs);

    String::Utf8Value logMessage(inspectResult->ToString());
    printf("%s\n", *logMessage);

    NanReturnUndefined();
}