#ifndef _WORK_ITEM_H_
#define _WORK_ITEM_H_

// node
#include <node.h>
#include <node_version.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

// C++
#include <string>
using namespace std;

// threadpool
#include "task_queue.h"

class WorkItem
{
    public:

        WorkItem(
            Handle<Function> callbackFunction,
            Handle<Object> workOptions,
            Handle<Object> calleeObject);

        virtual ~WorkItem();

        virtual void    InstanceWorkFunction() = 0;
        virtual void    InstanceWorkCallback() = 0;

        static void     Initialize(const char* moduleDir);

        static void*    WorkFunction(
            TASK_QUEUE_WORK_DATA *taskInfoPtr,
            void *threadContextPtr,
            void *workItemPtr);

        static void     WorkCallback(
            TASK_QUEUE_WORK_DATA *taskInfoPtr,
            void *threadContextPtr,
            void *workItemPtr);

        // these are public so they can be accessed in the 
        // uv_async callback
        Persistent<Function>    _CallbackFunction;
        Persistent<Object>      _WorkOptions;
        char*                   _Exception;
        char*                   _WorkResult;

        // __filename and __dirname to be set in context
        string                  _FileName;
        string                  _DirName;

        uint32_t                _ThreadId;
        string                  _ThreadPoolKey;

    private:

        void ProcessWorkOptions(Handle<Object> workOptions);

        static Persistent<Function> _Guid;
};

#endif /* _WORK_ITEM_H_ */