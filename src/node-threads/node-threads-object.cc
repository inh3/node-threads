#define _NODE_THREADS_OBJECT_CC_

#include "node-threads-object.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// Instance -------------------------------------------------------------------

NodeThreads::NodeThreads(string threadPoolKey, uint32_t numThreads)
{
    _ThreadPoolKey = threadPoolKey;
    _NumThreads = numThreads;
    printf("[ %s ] %u\n", _ThreadPoolKey.c_str(), _NumThreads);
}

NodeThreads::~NodeThreads()
{
    printf("NodeThreads::~NodeThreads\n");
}

// Node -----------------------------------------------------------------------

Persistent<Function> NodeThreads::Constructor;
Persistent<Function> NodeThreads::EventEmitter;
Persistent<Value> NodeThreads::NumCPUs;

NAN_METHOD(NodeThreads::New)
{
    NanScope();

    if (args.IsConstructCall())
    {
        // add instance function properties
        /*args.This()->Set(String::NewSymbol("testMethod"),
            FunctionTemplate::New(Test)->GetFunction());*/

        // inherit from EventEmitter
        // https://groups.google.com/d/msg/v8-users/6kSAbnUb-rQ/QPMMfqssx5AJ
        Local<Function> eventEmitter = NanPersistentToLocal(NodeThreads::EventEmitter);
        eventEmitter->Call(args.This(), 0, NULL);

        // get the unique name param of the node thread instance
        String::Utf8Value threadPoolName(args[0]->ToString());
        string threadPoolKey(*threadPoolName);

        // get number of threads
        Local<Value> numCpus = NanPersistentToLocal(NodeThreads::NumCPUs);
        uint32_t numThreads = numCpus->Uint32Value();
        if(args.Length() == 2)
        {
            numThreads = args[1]->Uint32Value();
        }

        // wrap the class and return the javascript object
        NodeThreads* nodeThread = new NodeThreads(threadPoolKey, numThreads);
        nodeThread->Wrap(args.This());
        NanReturnValue(args.This());
    }

    NanReturnUndefined();
}

NAN_GETTER(NodeThreads::GetThreadPoolKey)
{
    NanScope();
    NodeThreads* nodeThread = ObjectWrap::Unwrap<NodeThreads>(args.This());
    NanReturnValue(String::New(nodeThread->_ThreadPoolKey.c_str()));
}

NAN_GETTER(NodeThreads::GetNumThreads)
{
    NanScope();
    NodeThreads* nodeThread = ObjectWrap::Unwrap<NodeThreads>(args.This());
    NanReturnValue(Integer::New(nodeThread->_NumThreads));
}