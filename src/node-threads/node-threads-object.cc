#define _NODE_THREADS_OBJECT_CC_

#include "node-threads-object.h"

//using namespace node;
#include <v8.h>
using namespace v8;

#include "nan.h"

// custom
#include "thread.h"
#include "function-work-item.h"
#include "utilities.h"
#include "file_info.h"
#include "nt-environment.h"

Persistent<Function> NodeThreads::Constructor;

// Instance -------------------------------------------------------------------

NodeThreads::NodeThreads(
    string threadPoolKey,
    uint32_t numThreads,
    bool isWebWorker)
{
    _ThreadPoolKey = threadPoolKey;
    _NumThreads = numThreads;
    _Destroyed = false;
    _IsWebWorker = isWebWorker;

    printf("[ %s ] %u\n", _ThreadPoolKey.c_str(), _NumThreads);

    // create task queue and thread pool
    _TaskQueue = CreateTaskQueue(0);
    _ThreadPool = CreateThreadPool(
        _NumThreads,
        _TaskQueue,
        Thread::ThreadInit,
        Thread::ThreadPostInit,
        Thread::ThreadDestroy,
        (void*)this);
}

NodeThreads::~NodeThreads()
{
    printf("NodeThreads::~NodeThreads\n");

    if(!_Destroyed)
    {
        Destroy();
    }
}

void NodeThreads::Destroy()
{
    printf("NodeThreads::Destroy\n");

    // destroy thread pool and task queue
    _Destroyed = true;
    DestroyThreadPool(_ThreadPool);
    DestroyTaskQueue(_TaskQueue);
}

string NodeThreads::GetThreadPoolKey()
{
    return _ThreadPoolKey;
}

bool NodeThreads::IsWebWorker()
{
    return _IsWebWorker;
}

void NodeThreads::QueueWorkItem(void* workItem)
{
    // reference to task queue item to be added
    TASK_QUEUE_ITEM     *taskQueueItem = 0;

    // create task queue item object
    taskQueueItem = (TASK_QUEUE_ITEM*)malloc(sizeof(TASK_QUEUE_ITEM));
    memset(taskQueueItem, 0, sizeof(TASK_QUEUE_ITEM));

    // store reference to work item
    taskQueueItem->taskItemData = workItem;

    // set the task item work function
    taskQueueItem->taskItemFunction = WorkItem::WorkFunction;

    // set the task item callback function
    taskQueueItem->taskItemCallback = WorkItem::WorkCallback;

    // set the task item id (not used right now)
    taskQueueItem->taskId = 0;
    
    // add the task to the queue
    AddTaskToQueue(_TaskQueue, taskQueueItem);
}

void NodeThreads::QueueFunctionWorkItem(
    const char* functionString,
    Handle<Value> functionParam,
    Handle<Function> callbackFunction,
    Handle<Object> workOptions,
    Handle<Object> calleeObject,
    Handle<Object> nodeThreads)
{
    FunctionWorkItem* functionWorkItem = new FunctionWorkItem(
        functionString,
        functionParam,
        callbackFunction,
        workOptions,
        calleeObject,
        nodeThreads);

    QueueWorkItem((void*)functionWorkItem);
}

// Node -----------------------------------------------------------------------

NAN_METHOD(NodeThreads::New)
{
    NanScope();

    if (args.IsConstructCall())
    {
        // add instance function properties
        args.This()->Set(NanNew<String>("executeFunction"),
            NanNew<FunctionTemplate>(NodeThreads::ExecuteFunction)->GetFunction());

        // inherit from EventEmitter
        // https://groups.google.com/d/msg/v8-users/6kSAbnUb-rQ/QPMMfqssx5AJ
        Local<Function> eventEmitter = NanNew(NTEnvironment::EventEmitter);
        eventEmitter->Call(args.This(), 0, NULL);

        // get the unique name param of the node thread instance
        String::Utf8Value threadPoolName(args[0]->ToString());
        string threadPoolKey(*threadPoolName);

        // get number of threads
        Local<Value> numCpus = NanNew(NTEnvironment::NumCPUs);
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
    NanReturnValue(NanNew<String>(nodeThread->_ThreadPoolKey.c_str()));
}

NAN_GETTER(NodeThreads::GetNumThreads)
{
    NanScope();
    NodeThreads* nodeThread = ObjectWrap::Unwrap<NodeThreads>(args.This());
    NanReturnValue(NanNew<Uint32>(nodeThread->_NumThreads));
}

NAN_METHOD(NodeThreads::ExecuteFunction)
{
    NanScope();

    Handle<Object> calleeObject = GetCalleeInfo().As<Object>();

    Handle<String> funcStrHandle;
    Handle<Object> workOptions;

    if((args.Length() >= 1) 
        && (args[0]->IsFunction() || args[0]->IsString()))
    {
        // check first argument as function
        if(args[0]->IsFunction())
        {
            funcStrHandle = args[0].As<Function>()->ToString();
        }
        else if(args[0]->IsString())
        {
            funcStrHandle = args[0].As<String>();
        }

        // check if the context object is valid
        if((args.Length() == 4) && args[3]->IsObject())
        {
            workOptions = args[3]->ToObject();
        }
        else
        {
            workOptions = NanNew<Object>();
        }
    }
    
    if(funcStrHandle.IsEmpty())
    {
        ThrowException(Exception::TypeError(NanNew<String>("Invalid parameter(s) passed to 'executeFunction(...)'\n")));
        NanReturnUndefined();
    }
    else
    {
        NodeThreads* nodeThread = ObjectWrap::Unwrap<NodeThreads>(args.This());

        String::Utf8Value funcStrValue(funcStrHandle);
        nodeThread->QueueFunctionWorkItem(
            *funcStrValue,
            (args.Length() >= 2 ? args[1] : (Handle<Value>)Null()),
            args[2].As<Function>(),
            workOptions,
            calleeObject,
            args.This());
    }

    NanReturnValue(workOptions->Get(NanNew<String>("id")));
}

// node helpers ---------------------------------------------------------------

Handle<Value> NodeThreads::GetCalleeInfo()
{
    NanScope();

#if (NODE_MODULE_VERSION > 0x000B)
    Local<Function> strackTraceFunction = Local<Function>::New(
        Isolate::GetCurrent(),
        NTEnvironment::CalleeByStackTrace);
    Local<Value> pathModule = Local<Object>::New(
        Isolate::GetCurrent(),
        NTEnvironment::Path);
#else
    Local<Function> strackTraceFunction = Local<Function>::New(NTEnvironment::CalleeByStackTrace);
    Local<Value> pathModule = Local<Object>::New(NTEnvironment::Path);
#endif

    // get the __filename and __dirname properties
    Handle<Value> calleeObject = strackTraceFunction->Call(
        Context::GetCurrent()->Global(),
        1,
        &pathModule);

    return scope.Close(calleeObject);
}
