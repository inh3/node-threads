#ifndef _WORK_ITEM_H_
#define _WORK_ITEM_H_

// threadpool
#include "task_queue.h"

class WorkItem
{
    public:

        WorkItem();
        virtual ~WorkItem();

        virtual void*   InstanceWorkFunction() = 0;
        virtual void    InstanceWorkCallback() = 0;

        static void*    WorkFunction(
            TASK_QUEUE_WORK_DATA *taskInfoPtr,
            void *threadContextPtr,
            void *workItemPtr);

        static void     WorkCallback(
            TASK_QUEUE_WORK_DATA *taskInfoPtr,
            void *threadContextPtr,
            void *workItemPtr);
};

#endif /* _WORK_ITEM_H_ */