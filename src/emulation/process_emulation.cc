#define _PROCESS_EMULATION_CC_

#include "process_emulation.h"

// C
#include <stdio.h>

// custom
#include "thread.h"
#include "utilities.h"

string Process::Arch;
string Process::Platform;
string Process::Env;

void Process::Initialize()
{
    NanScope();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    Local<Object> nodeContext = isolate->GetCurrentContext()->Global();
#else
    Local<Object> nodeContext = Context::GetCurrent()->Global();
#endif

    Local<Object> processObject = nodeContext->Get(String::NewSymbol("process"))->ToObject();

    // store 'arch' and 'platform'
    String::Utf8Value processArch(
        processObject->Get(String::NewSymbol("arch"))->ToString());
    String::Utf8Value processPlatform(
        processObject->Get(String::NewSymbol("platform"))->ToString());

    Process::Arch.assign(*processArch);
    Process::Platform.assign(*processPlatform);

    // store reference to stringify
    Handle<Object> jsonObject = nodeContext->Get(v8::String::New("JSON"))->ToObject();
    Handle<Function> stringifyFunc = jsonObject->Get(v8::String::New("stringify")).As<Function>();

    // store stringified version of 'env' object
    Handle<Value> envValue = processObject->Get(String::NewSymbol("env"));
    Handle<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &(envValue));

    String::Utf8Value processEnv(stringifyResult->ToString());
    Process::Env.assign(*processEnv);
}

Handle<Object> Process::GetIsolateProcess()
{
    Handle<Object> processObject = Object::New();

    // set stdout.write
    Local<FunctionTemplate> stdOutWrite = FunctionTemplate::New(Process::StdOutWrite);
    Local<Function> stdOutWriteFunc = stdOutWrite->GetFunction();
    stdOutWriteFunc->SetName(String::NewSymbol("write"));
    Handle<Object> stdOutObject = Object::New();
    stdOutObject->Set(String::NewSymbol("write"), stdOutWriteFunc);
    processObject->Set(String::NewSymbol("stdout"), stdOutObject);

    // set stderr.write
    Local<FunctionTemplate> stdErrWrite = FunctionTemplate::New(Process::StdErrWrite);
    Local<Function> stdErrWriteFunc = stdErrWrite->GetFunction();
    stdErrWriteFunc->SetName(String::NewSymbol("write"));
    Handle<Object> stdErrObject = Object::New();
    stdErrObject->Set(String::NewSymbol("write"), stdErrWriteFunc);
    processObject->Set(String::NewSymbol("stderr"), stdErrObject);

    processObject->SetAccessor(
        String::NewSymbol("env"),
        Process::GetEnv,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    processObject->SetAccessor(
        String::NewSymbol("arch"),
        Process::GetArch,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    processObject->SetAccessor(
        String::NewSymbol("platform"),
        Process::GetPlatform,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    return processObject;
}

// node -----------------------------------------------------------------------

NAN_METHOD(Process::StdOutWrite)
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    String::Utf8Value outputStr(args[0]->ToString());
    fprintf(stdout, "%s", *outputStr);

    NanReturnUndefined();
}

NAN_METHOD(Process::StdErrWrite)
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    String::Utf8Value outputStr(args[0]->ToString());
    fprintf(stderr, "%s", *outputStr);

    NanReturnUndefined();
}

NAN_GETTER(Process::GetArch)
{
// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    NanReturnValue(String::New(Process::Arch.c_str()));
}

NAN_GETTER(Process::GetEnv)
{
// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    Local<Object> currentContext = isolate->GetCurrentContext()->Global();
#else
    HandleScope scope;
    Local<Object> currentContext = Context::GetCurrent()->Global();
#endif

    // store reference to parse
    Handle<Object> jsonObject = currentContext->Get(v8::String::New("JSON"))->ToObject();
    Handle<Function> parseFunc = jsonObject->Get(v8::String::New("parse")).As<Function>();

    Local<Value> jsonString = String::New(Process::Env.c_str());

    NanReturnValue(parseFunc->Call(jsonObject, 1, &jsonString));
}

NAN_GETTER(Process::GetPlatform)
{
// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    NanReturnValue(String::New(Process::Platform.c_str()));
}