#define _WORK_ITEM_CC_

#include "work-item.h"

// threadpool
#include "synchronize.h"

// custom
#include "thread.h"
#include "callback-manager.h"
#include "file_info.h"
#include "utilities.h"
#include "environment.h"

// callback manager
static CallbackManager* callbackManager = &(CallbackManager::GetInstance());

WorkItem::WorkItem(Handle<Object> threadPoolObject)
{
    printf("WorkItem::WorkItem\n");
    _WorkResult = NULL;
    _Exception = NULL;

#if (NODE_MODULE_VERSION > 0x000B)
    _ThreadPoolObject.Reset(Isolate::GetCurrent(), threadPoolObject);
#else
    _ThreadPoolObject = Persistent<Object>::New(threadPoolObject);
#endif
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

    _WorkOptions.Dispose();
    _WorkOptions.Clear();

    _ThreadPoolObject.Dispose();
    _ThreadPoolObject.Clear();
}

void WorkItem::AsyncCallback(
    Handle<Value> errorHandle,
    Handle<Value> infoHandle,
    Handle<Value> resultHandle)
{
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(Isolate::GetCurrent());
    Local<Object> threadPoolObject = Local<Object>::New(
            Isolate::GetCurrent(),
            _ThreadPoolObject);
#else
    HandleScope scope;
     Local<Object> threadPoolObject = Local<Object>::New(
            _ThreadPoolObject);
#endif

    Handle<Object> dataObject = Object::New();
    dataObject->Set(String::NewSymbol("error"), errorHandle);
    dataObject->Set(String::NewSymbol("info"), infoHandle);
    dataObject->Set(String::NewSymbol("result"), resultHandle);

    Handle<Value> eventHandle = Object::New();
    eventHandle.As<Object>()->Set(String::NewSymbol("data"), dataObject);

    Handle<Function> emitFunction = threadPoolObject->Get(
        String::NewSymbol("emit"))
    .As<Function>();

     // emit "message" on main thread
    Handle<Value> args[] = { 
        String::New("message"),
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

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
        HandleScope handleScope(isolate);
        Context::Scope contextScope(isolate, threadContext->isolate_context);
        Handle<Object> contextObject = isolate->GetCurrentContext()->Global();
#else
        HandleScope handleScope;
        Context::Scope contextScope(threadContext->isolate_context);
        Handle<Object> contextObject = Context::GetCurrent()->Global();
#endif

        workItem->InstanceWorkFunction(contextObject);
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
        // perform instance callback
        workItem->InstanceWorkCallback();

        // add to callback queue
        callbackManager->AddWorkItem(workItem);

        // send async to main thread
        thread_context_t* threadContext = (thread_context_t*)threadContextPtr;
        uv_async_send(threadContext->uv_async_ptr);
    }
}