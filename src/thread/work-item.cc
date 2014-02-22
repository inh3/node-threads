#define _WORK_ITEM_CC_

#include "work-item.h"

// custom
#include "thread.h"

WorkItem::WorkItem()
{

}

WorkItem::~WorkItem()
{

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

        workItem->InstanceWorkFunction(isolate);
    }

    // leave the isolate
    isolate->Exit();

    return workItem;
}

void WorkItem::WorkCallback(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    WorkItem* workItem = (WorkItem*)workItemPtr;
    workItem->InstanceWorkCallback();

    delete workItem;
}