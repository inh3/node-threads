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
    Local<Object> nodeContext = NanGetCurrentContext()->Global();
#endif

    Local<Object> processObject = nodeContext->Get(NanNew<String>("process"))->ToObject();

    // store 'arch' and 'platform'
    String::Utf8Value processArch(
        processObject->Get(NanNew<String>("arch"))->ToString());
    String::Utf8Value processPlatform(
        processObject->Get(NanNew<String>("platform"))->ToString());

    Process::Arch.assign(*processArch);
    Process::Platform.assign(*processPlatform);

    // store reference to stringify
    Handle<Object> jsonObject = nodeContext->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> stringifyFunc = jsonObject->Get(NanNew<String>("stringify")).As<Function>();

    // store stringified version of 'env' object
    Handle<Value> envValue = processObject->Get(NanNew<String>("env"));
    Handle<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &(envValue));

    String::Utf8Value processEnv(stringifyResult->ToString());
    Process::Env.assign(*processEnv);
}

Handle<Object> Process::GetIsolateProcess()
{
    Handle<Object> processObject = NanNew<Object>();

    // set stdout.write
    Local<FunctionTemplate> stdOutWrite = NanNew<FunctionTemplate>(Process::StdOutWrite);
    Local<Function> stdOutWriteFunc = stdOutWrite->GetFunction();
    stdOutWriteFunc->SetName(NanNew<String>("write"));
    Handle<Object> stdOutObject = NanNew<Object>();
    stdOutObject->Set(NanNew<String>("write"), stdOutWriteFunc);
    processObject->Set(NanNew<String>("stdout"), stdOutObject);

    // set stderr.write
    Local<FunctionTemplate> stdErrWrite = NanNew<FunctionTemplate>(Process::StdErrWrite);
    Local<Function> stdErrWriteFunc = stdErrWrite->GetFunction();
    stdErrWriteFunc->SetName(NanNew<String>("write"));
    Handle<Object> stdErrObject = NanNew<Object>();
    stdErrObject->Set(NanNew<String>("write"), stdErrWriteFunc);
    processObject->Set(NanNew<String>("stderr"), stdErrObject);

    processObject->SetAccessor(
        NanNew<String>("env"),
        Process::GetEnv,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    processObject->SetAccessor(
        NanNew<String>("arch"),
        Process::GetArch,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    processObject->SetAccessor(
        NanNew<String>("platform"),
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

    NanReturnValue(NanNew<String>(Process::Arch.c_str()));
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
    Local<Object> currentContext = NanGetCurrentContext()->Global();
#endif

    // store reference to parse
    Handle<Object> jsonObject = currentContext->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> parseFunc = jsonObject->Get(NanNew<String>("parse")).As<Function>();

    Local<Value> jsonString = NanNew<String>(Process::Env.c_str());

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

    NanReturnValue(NanNew<String>(Process::Platform.c_str()));
}
