#define _WORK_ITEM_CC_

#include "work-item.h"

// custom
#include "thread.h"
#include "callback-manager.h"

// callback manager
static CallbackManager* callbackManager = &(CallbackManager::GetInstance());

WorkItem::WorkItem(Handle<Function> callbackFunction)
{
    printf("WorkItem::WorkItem\n");
    _WorkResult = NULL;

#if (NODE_MODULE_VERSION > 0x000B)
    _CallbackFunction.Reset(Isolate::GetCurrent(), callbackFunction);
#else
    _CallbackFunction = Persistent<Function>::New(callbackFunction);
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

    _CallbackFunction.Dispose();
    _CallbackFunction.Clear();
}

void* WorkItem::WorkFunction(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    // get reference to the thread context
    thread_context_t* threadContext = (thread_context_t*)threadContextPtr;

    // get refrence to work item
    WorkItem* workItem = (WorkItem*)workItemPtr;

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

        // enter thread specific context
        Context::Scope contextScope(isolate, threadContext->isolate_context);
#else
        HandleScope handleScope;

        // enter thread specific context
        Context::Scope contextScope(threadContext->isolate_context);
#endif

        printf("[ Thread Pool Key ] %s\n", threadContext->nodeThreads->GetThreadPoolKey().c_str());
        workItem->InstanceWorkFunction();
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

        static int x = 0;
        printf("*** MAKING ASYNC CALLBACK! %u\n", ++x);
    }
}