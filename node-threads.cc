// v8 and node
#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// node threads
#include "node-threads-factory.h"

// custom
#include "thread-isolate.h"

/**
 * Gets or creates a thread pool instance.
 *
 * Gets a thread pool by name, or creates one if it doesn't exist.
 * 
 * Throws an exception if parameters are invalid and/or missing.
 * 
 * @param[in] threadPoolName    name Name of thread pool
 * @param[in] numThreads        Number of threads within thread pool
 *
 * @return An instance of a thread pool object
 */
NAN_METHOD(GetThreadPool)
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

    ThrowException(Exception::TypeError(String::New("Invalid parameter(s) passed to 'createThreadPool(...)'\n")));
    NanReturnUndefined();
}

/**
 * Deletes a thread pool instance.
 *
 * Throws an exception if parameter is invalid and/or missing.
 * 
 * @param[in] threadPoolName    name Name of thread pool
 *
 * @return True if a thread pool was deleted, false if not
 */
NAN_METHOD(DeleteThreadPool)
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

    ThrowException(Exception::TypeError(String::New("Invalid parameter passed to 'deleteThreadPool(...)'\n")));
    NanReturnUndefined();
}

// https://github.com/joyent/node/wiki/Api-changes-between-v0.8-and-v0.10 -----

NAN_METHOD(SubStack)
{
    NanScope();

    // get ref to the file and dir name of module
    String::Utf8Value dirString(args[0]->ToString());

    ThreadIsolate::Initialize(*dirString);

    Local<Object> nodeThreadsModule = Object::New();
    nodeThreadsModule->Set(NanSymbol("getThreadPool"),
        FunctionTemplate::New(GetThreadPool)->GetFunction());

    nodeThreadsModule->Set(NanSymbol("deleteThreadPool"),
        FunctionTemplate::New(DeleteThreadPool)->GetFunction());

    NanReturnValue(nodeThreadsModule);
}

void init(Handle<Object> exports, Handle<Object> module)
{
    NodeThreadsFactory::Init(exports, module);

    module->Set(String::NewSymbol("exports"),
        FunctionTemplate::New(SubStack)->GetFunction());
}

NODE_MODULE(node_threads, init);