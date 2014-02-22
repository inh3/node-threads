#define _WORK_ITEM_CC_

#include "work-item.h"

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
    WorkItem* workItem = (WorkItem*)workItemPtr;
    return workItem->InstanceWorkFunction();
}

void WorkItem::WorkCallback(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    WorkItem* workItem = (WorkItem*)workItemPtr;
    return workItem->InstanceWorkCallback();
}