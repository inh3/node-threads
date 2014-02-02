// v8 and node
#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// node threads
#include "node-threads-factory.h"

NAN_METHOD(CreateThreadPool)
{
    NanScope();

    // must be: 1-2 arguments
    // 1) thread pool name
    // 2) number of threads
    if((args.Length() > 2) || (args.Length() < 1) ||
        ((args.Length() == 2) && (!args[0]->IsString() || !args[1]->IsNumber())) ||
        ((args.Length() == 1) && !args[0]->IsString()))
    {
        ThrowException(Exception::TypeError(
            String::New("Invalid parameters passed to 'createThreadPool(...)'\n")));
        NanReturnUndefined();
    }

    // get reference to factory method
    Local<FunctionTemplate> instanceTemplate = FunctionTemplate::New(NodeThreadsFactory::NewInstance);
    Local<Function> instanceFunction = instanceTemplate->GetFunction();

    // create a new instance with parameters passed in
    Handle<Value> argv[args.Length()];
    for(int i = 0; i < args.Length(); i++) { argv[i] = args[i]; }
    Local<Value> newInstance = instanceFunction->Call(args.This(), args.Length(), argv);
    
    // return the new instance to caller
    NanReturnValue(newInstance);
}

void init(Handle<Object> exports, Handle<Object> module)
{
    NodeThreadsFactory::Init(exports, module);

    exports->Set(NanSymbol("createThreadPool"),
        FunctionTemplate::New(CreateThreadPool)->GetFunction());
}

NODE_MODULE(node_threads, init);