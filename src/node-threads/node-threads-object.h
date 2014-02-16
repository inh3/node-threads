#ifndef _NODE_THREADS_OBJECT_H_
#define _NODE_THREADS_OBJECT_H_

#include <string>
using namespace std;

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
using namespace v8;
using namespace node;

#include "nan.h"

// custom thread pool
#include "task_queue.h"
#include "thread_pool.h"

class NodeThreads : public ObjectWrap
{
    public:

        // constructor 'new'
        static NAN_METHOD(New);

        // property accessors
        static NAN_GETTER(GetThreadPoolKey);
        static NAN_GETTER(GetNumThreads);

        static Persistent<Function> Constructor;
        static Persistent<Function> EventEmitter;
        static Persistent<Value> NumCPUs;

        // provide public access to Node's ObjectWrap
        // Ref() and Unref() functions for use by the factory
        virtual void Ref() { ObjectWrap::Ref(); }
        virtual void Unref() { ObjectWrap::Unref(); }
    
    private:

        explicit NodeThreads(string threadPoolKey, uint32_t numThreads);
        ~NodeThreads();

        string                  _ThreadPoolKey;
        uint32_t                _NumThreads;

        THREAD_POOL_DATA        *_ThreadPool;
        TASK_QUEUE_DATA         *_TaskQueue;
};

#endif /* _NODE_THREADS_OBJECT_H_ */