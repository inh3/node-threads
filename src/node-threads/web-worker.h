#ifndef _WEB_WORKER_H_
#define _WEB_WORKER_H_

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

#include "node-threads-object.h"

class WebWorker : public NodeThreads
{
    public:

        // constructor 'new'
        static NAN_METHOD(New);

        // 'onmessage' property
        static NAN_GETTER(OnMessageGet);
        static NAN_SETTER(OnMessageSet);

        // js constructor
        static Persistent<Function> Constructor;
        static void Init();

        // provide public access to Node's ObjectWrap
        // Ref() and Unref() functions for use by the factory
        virtual void Ref() { ObjectWrap::Ref(); }
        virtual void Unref() { ObjectWrap::Unref(); }

    private:

        explicit WebWorker(const char* threadPoolKey, bool isFunction);
        virtual ~WebWorker();

        void QueueWebWorker(
            char* eventObject,
            Handle<Object> nodeThreads);

        Persistent<Function>    _OnMessage;

        bool                    _IsFunction;

        static NAN_METHOD(PostMessage);
        static NAN_METHOD(AddEventListener);
        static NAN_METHOD(Terminate);
};

#endif /* _WEB_WORKER_H_ */