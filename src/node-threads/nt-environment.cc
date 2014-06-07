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
Persistent<Uint32>      NTEnvironment::NumCPUs;

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
    Handle<NanUnboundScript> guidScript = NanNew<NanUnboundScript>(
        NanNew<String>(guidFile.FileContents()));
    Handle<Value> guidFunction = NanRunScript(guidScript);

    NanAssignPersistent(
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
    Handle<NanUnboundScript> stackScript = NanNew<NanUnboundScript>(
        NanNew<String>(stackFile.FileContents()));
    Handle<Value> stackTraceFunction = NanRunScript(stackScript);

    NanAssignPersistent(
        CalleeByStackTrace,
        stackTraceFunction.As<Function>());
}

void NTEnvironment::NodeInitialize()
{
    Handle<Object> envModule = NanNew(NTEnvironment::Module);

    // store reference to event emitter
    Local<Function> requireFunction = NanNew(
        envModule->Get(NanNew<String>("require")).As<Function>());
    Local<Value> args[] = { NanNew<String>("events") };
    Local<Object> eventsModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        NTEnvironment::EventEmitter, 
        eventsModule->Get(NanNew<String>("EventEmitter")).As<Function>());

    // store reference to path
    args[0] = NanNew<String>("path");
    Local<Object> pathModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        NTEnvironment::Path, 
        pathModule);

    // store reference to util
    args[0] = NanNew<String>("util");
    Local<Object> utilModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    NanAssignPersistent(
        NTEnvironment::Util, 
        utilModule);

    // store number of cpu cores
    args[0] = NanNew<String>("os");
    Local<Object> osModule = requireFunction->Call(
        envModule, 1, args)->ToObject();
    Local<Function> cpuFunction = osModule->Get(NanNew<String>("cpus")).As<Function>();
    Local<Array> cpuArray = cpuFunction->Call(
        envModule, 0, NULL).As<Array>();
    NanAssignPersistent(
        NTEnvironment::NumCPUs,
        NanNew<Uint32>((cpuArray->Length() > 2 ? cpuArray->Length() - 1 : 2)));
}
