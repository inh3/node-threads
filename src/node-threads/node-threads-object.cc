#define _NODE_THREADS_OBJECT_CC_

#include "node-threads-object.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"
#include "utilities.h"

// instance definitions

NodeThreads::NodeThreads() {}

NodeThreads::~NodeThreads() {}

// static definitions

Persistent<Function> NodeThreads::Constructor;
Persistent<Function> NodeThreads::EventEmitter;

void NodeThreads::Init(Handle<Object> exports, Handle<Object> module)
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

    // prepare the function template    
    Local<FunctionTemplate> functionTemplate = FunctionTemplate::New(New);
    functionTemplate->SetClassName(String::NewSymbol("NodeThreads"));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // inherit from event emitter
    args[0] = { String::New("util") };
    Local<Object> utilModule = requireFunction->Call(module, 1, args)->ToObject();
    Local<Function> inheritsFunction = utilModule->Get(String::NewSymbol("inherits")).As<Function>();
    Local<Value> inheritArgs[] = { functionTemplate->GetFunction(), NanPersistentToLocal(NodeThreads::EventEmitter) };
    inheritsFunction->Call(module, 2, inheritArgs);

    // expose the constructor
    NanAssignPersistent(Function, NodeThreads::Constructor, functionTemplate->GetFunction());
}

NAN_METHOD(NodeThreads::New)
{
    NanScope();

    if (args.IsConstructCall())
    {
        // add instance function properties
        args.This()->Set(String::NewSymbol("testMethod"),
            FunctionTemplate::New(Test)->GetFunction());

        // inherit from EventEmitter
        // https://groups.google.com/d/msg/v8-users/6kSAbnUb-rQ/QPMMfqssx5AJ
        Local<Function> eventEmitter = NanPersistentToLocal(NodeThreads::EventEmitter);
        eventEmitter->Call(args.This(), 0, NULL);

        // wrap the class and return the javascript object
        NodeThreads* nodeThreads = new NodeThreads();
        nodeThreads->Wrap(args.This());
        NanReturnValue(args.This());
    }

    NanReturnUndefined();
}

NAN_METHOD(NodeThreads::NewInstance)
{
    NanScope();

    const unsigned argc = 1;
    Handle<Value> argv[argc] = { args[0] };
    Local<Object> newInstance = NanPersistentToLocal(NodeThreads::Constructor)->NewInstance(argc, argv);

    NanReturnValue(newInstance);
}

NAN_METHOD(NodeThreads::Test)
{
    NanScope();

    Local<String> testString = NanNewLocal<String>(String::New("NodeThreads::Test"));

    NanReturnValue(testString);
}