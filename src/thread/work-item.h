#ifndef _WORK_ITEM_H_
#define _WORK_ITEM_H_

// node
#include <node.h>
#include <node_version.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

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

    protected:

        Persistent<Function>    _CallbackFunction;
        char*                   _WorkResult;
};

#endif /* _WORK_ITEM_H_ */