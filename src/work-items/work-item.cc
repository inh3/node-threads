#define _WORK_ITEM_CC_

#include "work-item.h"

// threadpool
#include "synchronize.h"

// custom
#include "thread.h"
#include "callback-manager.h"
#include "file_info.h"
#include "utilities.h"
#include "nt-environment.h"

// callback manager
static CallbackManager* callbackManager = &(CallbackManager::GetInstance());

WorkItem::WorkItem(Handle<Object> threadPoolObject)
{
    printf("WorkItem::WorkItem\n");
    _WorkResult = NULL;
    _Exception = NULL;

    NanAssignPersistent(_ThreadPoolObject, threadPoolObject);

    _AsyncShouldProcess = false;
}

// 'delete' can only be called from main thread
WorkItem::~WorkItem()
{
    static int x = 0;
    printf("WorkItem::~WorkItem - %u\n", ++x);

    if(_WorkResult != NULL)
    {
        free(_WorkResult);
    }

    if(_Exception != NULL)
    {
        free(_Exception);
    }

    NanDisposePersistent(_WorkOptions);
    NanDisposePersistent(_ThreadPoolObject);
}

void WorkItem::AsyncCallback(
    Handle<Value> errorHandle,
    Handle<Value> infoHandle,
    Handle<Value> resultHandle)
{
    NanScope();

    Local<Object> threadPoolObject = NanNew(_ThreadPoolObject);

    Handle<Object> dataObject = NanNew<Object>();
    dataObject->Set(NanNew<String>("error"), errorHandle);
    dataObject->Set(NanNew<String>("info"), infoHandle);
    dataObject->Set(NanNew<String>("result"), resultHandle);

    Handle<Value> eventHandle = NanNew<Object>();
    eventHandle.As<Object>()->Set(NanNew<String>("data"), dataObject);

    Handle<Function> emitFunction = threadPoolObject->Get(
        NanNew<String>("emit"))
    .As<Function>();

     // emit "message" on main thread
    Handle<Value> args[] = { 
        NanNew<String>("message"),
        eventHandle
    };
    emitFunction->Call(threadPoolObject, 2, args);
}

// static methods -------------------------------------------------------------

void* WorkItem::WorkFunction(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    // get reference to the thread context
    thread_context_t* threadContext = (thread_context_t*)threadContextPtr;

    // get refrence to work item
    WorkItem* workItem = (WorkItem*)workItemPtr;

    // get currently running thread id
    workItem->_ThreadId = SyncGetThreadId();
    workItem->_ThreadPoolKey.assign(
        threadContext->nodeThreads->GetThreadPoolKey().c_str());

    // get reference to thread isolate
    Isolate* isolate = threadContext->thread_isolate;
    {
        // lock the isolate
        Locker isolateLocker(isolate);

        // enter the isolate
        isolate->Enter();

        NanScope();

        Handle<Context> isolateContext = NanNew(threadContext->isolate_context);
        isolateContext->Enter();

        Handle<Object> contextObject = NanGetCurrentContext()->Global();

        workItem->InstanceWorkFunction(contextObject);

        isolateContext->Exit();
    }

    // leave the isolate
    isolate->Exit();

    return (void*)workItem;
}

void WorkItem::WorkCallback(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    WorkItem* workItem = (WorkItem*)workItemPtr;

    static int x = 0;
    printf("WorkItem::WorkCallback - %u\n", ++x);

    // context will be null if this is due to the thread pool
    // being destroyed, so don't execute the actual callback function
    // this will only occur on main thread since thread pool is destroying
    if(threadContextPtr == NULL)
    {
        delete workItem;
    }
    else
    {
        // add to callback queue
        callbackManager->AddWorkItem(workItem);

        // send async to main thread
        thread_context_t* threadContext = (thread_context_t*)threadContextPtr;
        uv_async_send(threadContext->uv_async_ptr);
    }
}
