#define _ENVIRONMENT_C_

#include "environment.h"

// custom
#include "thread-isolate.h"
#include "work-item.h"
#include "require.h"

// static member variables

Persistent<Value>       Environment::NumCPUs;
Persistent<Object>      Environment::Path;
Persistent<Function>    Environment::EventEmitter;
Persistent<Function>    Environment::CalleeByStackTrace;
Persistent<Function>    Environment::Guid;

// public methods --------------------------------------------------------------

void Environment::Initialize(const char* moduleDir, const char* processDir)
{
    GuidInitialize(moduleDir);
    StackTraceInitialize(moduleDir);

    Require::ModuleDir.assign(moduleDir);

    ThreadIsolate::Initialize(processDir);
}

// private methods -------------------------------------------------------------

void Environment::GuidInitialize(const char* moduleDir)
{
    NanScope();

    string guidPath;
    guidPath.assign(moduleDir);
    guidPath.append("/src/js/guid.js");
    FileInfo guidFile(guidPath.c_str());
    Handle<Script> guidScript = Script::New(
        String::New(guidFile.FileContents()),
        String::New("guid"));
    Handle<Value> guidFunction = guidScript->Run();

    NanAssignPersistent(
        Function,
        Guid,
        guidFunction.As<Function>());
}

void Environment::StackTraceInitialize(const char* moduleDir)
{
    NanScope();

    string stackPath;
    stackPath.assign(moduleDir);
    stackPath.append("/src/js/stack.js");
    FileInfo stackFile(stackPath.c_str());
    Handle<Script> stackScript = Script::New(
        String::New(stackFile.FileContents()),
        String::New("stack"));
    Handle<Value> stackTraceFunction = stackScript->Run();

    NanAssignPersistent(
        Function,
        CalleeByStackTrace,
        stackTraceFunction.As<Function>());
}