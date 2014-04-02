#define _ENVIRONMENT_C_

#include "environment.h"

// custom
#include "thread-isolate.h"
#include "work-item.h"
#include "require.h"
#include "process_emulation.h"

// static member variables

bool                    Environment::_IsInitialized = false;

Persistent<Value>       Environment::NumCPUs;
Persistent<Object>      Environment::Path;
Persistent<Function>    Environment::EventEmitter;
Persistent<Function>    Environment::CalleeByStackTrace;
Persistent<Function>    Environment::Guid;

string                  Environment::ModuleDir;
string                  Environment::ProcessDir;

// public methods --------------------------------------------------------------

void Environment::Initialize(const char* moduleDir, const char* processDir)
{
    // make sure to only initialize once
    if(_IsInitialized == false)
    {
        _IsInitialized = true;

        ModuleDir.assign(moduleDir);
        ProcessDir.assign(processDir);

        GuidInitialize(moduleDir);
        StackTraceInitialize(moduleDir);

        Process::Initialize();
    }
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