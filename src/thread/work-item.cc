#define _WORK_ITEM_CC_

#include "work-item.h"

// custom
#include "thread.h"

WorkItem::WorkItem()
{
    printf("WorkItem::WorkItem\n");
    _WorkResult = NULL;
}

WorkItem::~WorkItem()
{
    printf("WorkItem::~WorkItem\n");

    if(_WorkResult != NULL)
    {
        free(_WorkResult);
    }
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
    if(threadContextPtr != NULL)
    {
        workItem->InstanceWorkCallback();
    }

    delete workItem;
}