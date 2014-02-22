#define _NODE_THREADS_OBJECT_CC_

#include "node-threads-object.h"

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

// custom
#include "thread.h"
#include "work-item.h"
#include "function-work-item.h"
#include "utilities.h"

// Instance -------------------------------------------------------------------

NodeThreads::NodeThreads(string threadPoolKey, uint32_t numThreads)
{
    _ThreadPoolKey = threadPoolKey;
    _NumThreads = numThreads;
    _Destroyed = false;
    printf("[ %s ] %u\n", _ThreadPoolKey.c_str(), _NumThreads);

    // create task queue and thread pool
    _TaskQueue = CreateTaskQueue(0);
    _ThreadPool = CreateThreadPool(
        _NumThreads,
        _TaskQueue,
        Thread::ThreadInit,
        Thread::ThreadPostInit,
        Thread::ThreadDestroy);
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

void NodeThreads::QueueFunctionWorkItem()
{
    FunctionWorkItem* functionWorkItem = new FunctionWorkItem();

    // reference to task queue item to be added
    TASK_QUEUE_ITEM     *taskQueueItem = 0;

    // create task queue item object
    taskQueueItem = (TASK_QUEUE_ITEM*)malloc(sizeof(TASK_QUEUE_ITEM));
    memset(taskQueueItem, 0, sizeof(TASK_QUEUE_ITEM));

    // store reference to work item
    taskQueueItem->taskItemData = (void*)functionWorkItem;

    // set the task item work function
    taskQueueItem->taskItemFunction = WorkItem::WorkFunction;

    // set the task item callback function
    taskQueueItem->taskItemCallback = WorkItem::WorkCallback;

    // set the task item id (not used right now)
    taskQueueItem->taskId = 0;
    
    // add the task to the queue
    AddTaskToQueue(_TaskQueue, taskQueueItem);
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
        args.This()->Set(String::NewSymbol("executeFunction"),
            FunctionTemplate::New(NodeThreads::ExecuteFunction)->GetFunction());

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
    NanReturnValue(Uint32::NewFromUnsigned(nodeThread->_NumThreads));
}

NAN_METHOD(NodeThreads::ExecuteFunction)
{
    NanScope();

    Handle<Function> functionToExecute;

    if((args.Length() == 2) 
        && (args[0]->IsFunction() || args[0]->IsString())
        && (args[1]->IsFunction()))
    {
        printf("Good parameters!\n");
        NodeThreads* nodeThread = ObjectWrap::Unwrap<NodeThreads>(args.This());
        nodeThread->QueueFunctionWorkItem();
    }
    else
    {
        ThrowException(Exception::TypeError(String::New("Invalid parameter(s) passed to 'executeFunction(...)'\n")));
    }

    NanReturnUndefined();
}