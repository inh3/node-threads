#ifndef _WEB_WORKER_H_
#define _WEB_WORKER_H_

#include <node.h>
using namespace node;
#include <v8.h>
using namespace v8;

#include "nan.h"

// C++
#include <string>
using namespace std;

// custom
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

        // this is the function that executes in background thread
        Persistent<Function>    _MessageFunction;

    private:

        explicit WebWorker(const char* threadPoolKey, bool isFunction);
        virtual ~WebWorker();

        void QueueWebWorker(
            char* eventObject,
            char* workerScript,
            Handle<Object> nodeThreads);

        char* ProcessFunction(Handle<Function> workerFunction);

        Persistent<Function>    _OnMessage;
        bool                    _IsFunction;

        // __filename and __dirname to be set in context
        string                  _FileName;
        string                  _DirName;

        static NAN_METHOD(PostMessage);
        static NAN_METHOD(AddEventListener);
        static NAN_METHOD(Terminate);
};

#endif /* _WEB_WORKER_H_ */