// v8 and node
#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// node threads object
#include "node-threads-object.h"

NAN_METHOD(CreateThreadPool)
{
    NanScope();

    Local<FunctionTemplate> instanceTemplate = FunctionTemplate::New(NodeThreads::NewInstance);
    Local<Function> instanceFunction = instanceTemplate->GetFunction();
    Local<Value> newInstance = instanceFunction->Call(args.This(), 0, NULL);
    NanReturnValue(newInstance);
}

void init(Handle<Object> exports, Handle<Object> module)
{
    NodeThreads::Init(exports, module);

    exports->Set(NanSymbol("createThreadPool"),
        FunctionTemplate::New(CreateThreadPool)->GetFunction());
}

NODE_MODULE(node_threads, init);