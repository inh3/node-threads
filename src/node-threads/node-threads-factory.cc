#define _NODE_THREADS_FACTORY_CC_

#include "node-threads-factory.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// node-threads
#include "utilities.h"
#include "node-threads-object.h"
#include "environment.h"

NodeThreadMap NodeThreadsFactory::_NodeThreadMap;

void NodeThreadsFactory::Init(Handle<Object> exports, Handle<Object> module)
{
    NanScope();

    // store reference to event emitter
    Local<Function> requireFunction = NanNewLocal<Function>(module->Get(String::NewSymbol("require")).As<Function>());
    Local<Value> args[] = { String::New("events") };
    Local<Object> eventsModule = requireFunction->Call(module, 1, args)->ToObject();
    NanAssignPersistent(
        Function,
        Environment::EventEmitter, 
        eventsModule->Get(String::NewSymbol("EventEmitter")).As<Function>());

    // store reference to path
    args[0] = String::New("path");
    Local<Object> pathModule = requireFunction->Call(module, 1, args)->ToObject();
    NanAssignPersistent(
        Object,
        Environment::Path, 
        pathModule);

    // store number of cpu cores
    args[0] = String::New("os");
    Local<Object> osModule = requireFunction->Call(module, 1, args)->ToObject();
    Local<Function> cpuFunction = osModule->Get(String::NewSymbol("cpus")).As<Function>();
    Local<Array> cpuArray = cpuFunction->Call(module, 0, NULL).As<Array>();
    NanAssignPersistent(
        Value,
        Environment::NumCPUs,
        Uint32::NewFromUnsigned((cpuArray->Length() > 2 ? cpuArray->Length() - 1 : 2)));

    // prepare the constructor function template
    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New(NodeThreads::New);
    constructorTemplate->SetClassName(String::NewSymbol("NodeThread"));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // set accessors for read-only properties
    // http://v8.googlecode.com/svn/trunk/test/cctest/test-accessors.cc
    constructorTemplate->InstanceTemplate()->SetAccessor(
        String::NewSymbol("threadPoolKey"),
        NodeThreads::GetThreadPoolKey,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    constructorTemplate->InstanceTemplate()->SetAccessor(
        String::NewSymbol("numThreads"),
        NodeThreads::GetNumThreads,
        0,
        Handle<Value>(),
        v8::PROHIBITS_OVERWRITING,
        v8::ReadOnly);

    // inherit from event emitter
    args[0] = String::New("util");
    Local<Object> utilModule = requireFunction->Call(module, 1, args)->ToObject();
    Local<Function> inheritsFunction = utilModule->Get(String::NewSymbol("inherits")).As<Function>();
    Local<Value> inheritArgs[] = { 
        constructorTemplate->GetFunction(),
        NanPersistentToLocal(Environment::EventEmitter)
    };
    inheritsFunction->Call(module, 2, inheritArgs);

    // expose the constructor
    NanAssignPersistent(Function, NodeThreads::Constructor, constructorTemplate->GetFunction());
}

NAN_METHOD(NodeThreadsFactory::CreateInstance)
{
    NanScope();

    // get the unique name param of the node thread instance
    String::Utf8Value threadPoolName(args[0]->ToString());
    string nodeThreadKey(*threadPoolName);

    // attempt to find the node thread instance by its name
    NodeThreadMap::const_iterator nodeThreadInstance =
        NodeThreadsFactory::_NodeThreadMap.find(nodeThreadKey);

    // node thread instance has already been created
    if(nodeThreadInstance != NodeThreadsFactory::_NodeThreadMap.end())
    {
        NanReturnValue(NanObjectWrapHandle(nodeThreadInstance->second));
    }

    // can have up to two arguments
    uint32_t argc = 0;
    Handle<Value> argv[2];
    for(int i = 0; i < args.Length(); i++)
    {
        argv[i] = args[i];
        argc++;
    }

    // create instance
    Local<Object> newInstance = NanPersistentToLocal(NodeThreads::Constructor)->NewInstance(argc, argv);

    // add the node threads instance to the hash map for lookup next time
    NodeThreads *nodeThread = ObjectWrap::Unwrap<NodeThreads>(newInstance);
    nodeThread->Ref();
    NodeThreadsFactory::_NodeThreadMap.insert(make_pair(nodeThreadKey, nodeThread));

    NanReturnValue(newInstance);
}

NAN_METHOD(NodeThreadsFactory::DestroyInstance)
{
    NanScope();

    // get the unique name param of the node thread instance
    String::Utf8Value threadPoolName(args[0]->ToString());
    string nodeThreadKey(*threadPoolName);

    // attempt to find the node thread instance by its name
    NodeThreadMap::const_iterator nodeThreadInstance =
        NodeThreadsFactory::_NodeThreadMap.find(nodeThreadKey);

    // node thread instance exists
    if(nodeThreadInstance != NodeThreadsFactory::_NodeThreadMap.end())
    {
        // remove the instance and unreference it
        NodeThreads *nodeThread = nodeThreadInstance->second;
        NodeThreadsFactory::_NodeThreadMap.erase(nodeThreadKey);
        nodeThread->Destroy();
        nodeThread->Unref();
        NanReturnValue(Boolean::New(true));
    }

    NanReturnValue(Boolean::New(false));
}