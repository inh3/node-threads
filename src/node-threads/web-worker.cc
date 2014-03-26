#define _WEB_WORKER_C_

#include "web-worker.h"

// Globals

extern Persistent<Function> EventEmitter;
extern Persistent<Object> Path;
extern Persistent<Value> NumCPUs;
extern Persistent<Function> CalleeByStackTrace;

WebWorker::WebWorker()
{
    printf("WebWorker::WebWorker\n");
}

WebWorker::~WebWorker()
{
    printf("WebWorker::~WebWorker\n");
}

// Node -----------------------------------------------------------------------

NAN_METHOD(WebWorker::New)
{
    NanScope();

    if (args.IsConstructCall())
    {

        // inherit from EventEmitter
        // https://groups.google.com/d/msg/v8-users/6kSAbnUb-rQ/QPMMfqssx5AJ
        Local<Function> eventEmitter = NanPersistentToLocal(EventEmitter);
        eventEmitter->Call(args.This(), 0, NULL);

        // wrap the class and return the javascript object
        WebWorker* webWorker = new WebWorker();
        webWorker->Wrap(args.This());
        NanReturnValue(args.This());
    }

    NanReturnUndefined();
}