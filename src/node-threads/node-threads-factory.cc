#define _NODE_THREADS_FACTORY_CC_

#include "node-threads-factory.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

#include "utilities.h"
#include "node-threads-object.h"

void NodeThreadsFactory::Init(Handle<Object> exports, Handle<Object> module)
{
    NanScope();

    // store reference to event emitter
    Local<Function> requireFunction = NanNewLocal<Function>(module->Get(String::NewSymbol("require")).As<Function>());    
    Local<Value> args[] = { String::New("events") };
    Local<Object> eventsModule = requireFunction->Call(module, 1, args)->ToObject();
    NanAssignPersistent(
        Function,
        NodeThreads::EventEmitter, 
        eventsModule->Get(String::NewSymbol("EventEmitter")).As<Function>());

    // prepare the constructor function template    
    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New(NodeThreads::New);
    constructorTemplate->SetClassName(String::NewSymbol("NodeThreads"));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // inherit from event emitter
    args[0] = { String::New("util") };
    Local<Object> utilModule = requireFunction->Call(module, 1, args)->ToObject();
    Local<Function> inheritsFunction = utilModule->Get(String::NewSymbol("inherits")).As<Function>();
    Local<Value> inheritArgs[] = { constructorTemplate->GetFunction(), NanPersistentToLocal(NodeThreads::EventEmitter) };
    inheritsFunction->Call(module, 2, inheritArgs);

    // expose the constructor
    NanAssignPersistent(Function, NodeThreads::Constructor, constructorTemplate->GetFunction());
}

NAN_METHOD(NodeThreadsFactory::NewInstance)
{
    NanScope();

    const unsigned argc = 1;
    Handle<Value> argv[argc] = { args[0] };
    Local<Object> newInstance = NanPersistentToLocal(NodeThreads::Constructor)->NewInstance(argc, argv);

    NanReturnValue(newInstance);
}