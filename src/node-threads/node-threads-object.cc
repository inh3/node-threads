#define _NODE_THREADS_OBJECT_CC_

#include "node-threads-object.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// Instance -------------------------------------------------------------------

NodeThreads::NodeThreads() {}

NodeThreads::~NodeThreads()
{
    printf("NodeThreads::~NodeThreads\n");
}

// Node -----------------------------------------------------------------------

Persistent<Function> NodeThreads::Constructor;
Persistent<Function> NodeThreads::EventEmitter;

NAN_METHOD(NodeThreads::New)
{
    NanScope();

    printf("***********\n");

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

NAN_METHOD(NodeThreads::Test)
{
    NanScope();

    Local<String> testString = NanNewLocal<String>(String::New("NodeThreads::Test"));

    NanReturnValue(testString);
}