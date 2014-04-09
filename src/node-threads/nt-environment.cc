#define _NTENVIRONMENT_C_

#include "nt-environment.h"

// custom
#include "thread-isolate.h"
#include "work-item.h"
#include "require.h"
#include "process_emulation.h"

// static member variables -----------------------------------------------------

bool                    NTEnvironment::_IsInitialized = false;

// overall module global references
Persistent<Object>      NTEnvironment::Exports;
Persistent<Object>      NTEnvironment::Module;

// node global references
Persistent<Function>    NTEnvironment::EventEmitter;
Persistent<Object>      NTEnvironment::Path;
Persistent<Object>      NTEnvironment::Util;

// custom global references
Persistent<Function>    NTEnvironment::CalleeByStackTrace;
Persistent<Function>    NTEnvironment::Guid;
Persistent<Value>       NTEnvironment::NumCPUs;

string                  NTEnvironment::ModuleDir;
string                  NTEnvironment::ProcessDir;

// public methods --------------------------------------------------------------

void NTEnvironment::Initialize(const char* moduleDir, const char* processDir)
{
    // make sure to only initialize once
    if(_IsInitialized == false)
    {
        _IsInitialized = true;

        ModuleDir.assign(moduleDir);
        ProcessDir.assign(processDir);

        GuidInitialize(moduleDir);
        StackTraceInitialize(moduleDir);

        NodeInitialize();

        Process::Initialize();
    }
}

// private methods -------------------------------------------------------------

void NTEnvironment::GuidInitialize(const char* moduleDir)
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

void NTEnvironment::StackTraceInitialize(const char* moduleDir)
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

void NTEnvironment::NodeInitialize()
{
    Handle<Object> envModule = NanPersistentToLocal(
        NTEnvironment::Module).As<Object>();

    // store reference to event emitter
    Local<Function> requireFunction = NanNewLocal<Function>(
        envModule->Get(String::NewSymbol("require")).As<Function>());
    Local<Value> args[] = { String::New("events") };
    Local<Object> eventsModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        Function,
        NTEnvironment::EventEmitter, 
        eventsModule->Get(String::NewSymbol("EventEmitter")).As<Function>());

    // store reference to path
    args[0] = String::New("path");
    Local<Object> pathModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        Object,
        NTEnvironment::Path, 
        pathModule);

    // store reference to util
    args[0] = String::New("util");
    Local<Object> utilModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        Object,
        NTEnvironment::Util, 
        utilModule);

    // store number of cpu cores
    args[0] = String::New("os");
    Local<Object> osModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    Local<Function> cpuFunction = osModule->Get(String::NewSymbol("cpus")).As<Function>();
    Local<Array> cpuArray = cpuFunction->Call(
        envModule, 0, NULL).As<Array>();
    NanAssignPersistent(
        Value,
        NTEnvironment::NumCPUs,
        Uint32::NewFromUnsigned((cpuArray->Length() > 2 ? cpuArray->Length() - 1 : 2)));
}