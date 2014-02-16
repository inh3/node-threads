// v8 and node
#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// node threads
#include "node-threads-factory.h"

/**
 * Creates a thread pool instance.
 *
 * Throws an exception if parameters are invalid and/or missing.
 * 
 * @param[in] threadPoolName    name Name of thread pool
 * @param[in] numThreads        Number of threads within thread pool
 *
 * @return Instance of a thread pool object
 */
NAN_METHOD(CreateThreadPool)
{
    NanScope();

    // verify parameters being passed to function
    if((args.Length() == 2 && args[0]->IsString() && args[1]->IsNumber()) ||
       (args.Length() == 1 && args[0]->IsString()))
    {
        // get reference to factory method
        Local<FunctionTemplate> instanceTemplate = FunctionTemplate::New(NodeThreadsFactory::CreateInstance);
        Local<Function> instanceFunction = instanceTemplate->GetFunction();

        // create a new instance with parameters passed in
        Handle<Value> argv[] = { args[0], args[1] };
        Local<Value> newInstance = instanceFunction->Call(args.This(), args.Length(), argv);
        
        // return the new instance to caller
        NanReturnValue(newInstance);
    }

    ThrowException(Exception::TypeError(String::New("Invalid parameters passed to 'createThreadPool(...)'\n")));
    NanReturnUndefined();
}

/**
 * Destroys a thread pool instance
 *
 * Throws an exception if parameter is invalid and/or missing.
 * 
 * @param[in] threadPoolName    name Name of thread pool
 *
 * @return Thread pool instance
 */
NAN_METHOD(DestroyThreadPool)
{
    NanScope();

    // verify parameters being passed to function
    if(args.Length() == 1 && args[0]->IsString())
    {
        // get reference to factory method
        Local<FunctionTemplate> destroyTemplate = FunctionTemplate::New(NodeThreadsFactory::DestroyInstance);
        Local<Function> destroyFunction = destroyTemplate->GetFunction();

        // create a new instance with parameters passed in
        Handle<Value> argv[] = { args[0] };
        Local<Value> threadPoolDestroyed = destroyFunction->Call(args.This(), args.Length(), argv);

        NanReturnValue(threadPoolDestroyed);
    }

    ThrowException(Exception::TypeError(String::New("Invalid parameter passed to 'destroyThreadPool(...)'\n")));
    NanReturnUndefined();
}

void init(Handle<Object> exports, Handle<Object> module)
{
    NodeThreadsFactory::Init(exports, module);

    exports->Set(NanSymbol("createThreadPool"),
        FunctionTemplate::New(CreateThreadPool)->GetFunction());

    exports->Set(NanSymbol("destroyThreadPool"),
        FunctionTemplate::New(DestroyThreadPool)->GetFunction());
}

NODE_MODULE(node_threads, init);