// v8 and node
#include <v8.h>
using namespace v8;

#include "nan.h"

// node threads
#include "node-threads-factory.h"
#include "node-threads-object.h"
#include "web-worker.h"

// custom
#include "nt-environment.h"

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
        Local<FunctionTemplate> instanceTemplate = NanNew<FunctionTemplate>(NodeThreadsFactory::CreateInstance);
        Local<Function> instanceFunction = instanceTemplate->GetFunction();

        // create a new instance with parameters passed in
        Handle<Value> argv[] = { args[0], args[1] };
        Local<Value> newInstance = instanceFunction->Call(args.This(), args.Length(), argv);
        
        // return the new instance to caller
        NanReturnValue(newInstance);
    }

    NanThrowTypeError("Invalid parameter(s) passed to 'createThreadPool(...)'\n");
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
        Local<FunctionTemplate> destroyTemplate = NanNew<FunctionTemplate>(NodeThreadsFactory::DestroyInstance);
        Local<Function> destroyFunction = destroyTemplate->GetFunction();

        // create a new instance with parameters passed in
        Handle<Value> argv[] = { args[0] };
        Local<Value> threadPoolDestroyed = destroyFunction->Call(args.This(), args.Length(), argv);

        NanReturnValue(threadPoolDestroyed);
    }

    NanThrowTypeError("Invalid parameter passed to 'deleteThreadPool(...)'\n");
    NanReturnUndefined();
}

// https://github.com/joyent/node/wiki/Api-changes-between-v0.8-and-v0.10 -----

NAN_METHOD(SubStack)
{
    NanScope();

    // get ref to the module and process directory
    String::Utf8Value dirString(args[0]->ToString());
    String::Utf8Value processDirString(args[1]->ToString());

    // initialize environment
    NTEnvironment::Initialize(*dirString, *processDirString);

    NodeThreadsFactory::Init();
    WebWorker::Init();

    Local<Object> nodeThreadsModule = NanNew<Object>();
    nodeThreadsModule->Set(NanNew<String>("getThreadPool"),
        NanNew<FunctionTemplate>(GetThreadPool)->GetFunction());

    nodeThreadsModule->Set(NanNew<String>("deleteThreadPool"),
        NanNew<FunctionTemplate>(DeleteThreadPool)->GetFunction());

    nodeThreadsModule->Set(NanNew<String>("Worker"),
        NanNew(WebWorker::Constructor));

    NanReturnValue(nodeThreadsModule);
}

void init(Handle<Object> exports, Handle<Object> module)
{
    NanAssignPersistent(
        NTEnvironment::Exports,
        exports);

    NanAssignPersistent(
        NTEnvironment::Module,
        module);

    module->Set(NanNew<String>("exports"),
        NanNew<FunctionTemplate>(SubStack)->GetFunction());
}

NODE_MODULE(node_threads, init);
