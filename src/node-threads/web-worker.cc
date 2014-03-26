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

    _OnMessage.Dispose();
    _OnMessage.Clear();
}

// Node -----------------------------------------------------------------------

NAN_METHOD(WebWorker::New)
{
    NanScope();

    if (args.IsConstructCall())
    {
        // add instance function properties
        args.This()->Set(String::NewSymbol("postMessage"),
            FunctionTemplate::New(WebWorker::PostMessage)->GetFunction());
        args.This()->Set(String::NewSymbol("addEventListener"),
            FunctionTemplate::New(WebWorker::AddEventListener)->GetFunction());
        args.This()->Set(String::NewSymbol("terminate"),
            FunctionTemplate::New(WebWorker::Terminate)->GetFunction());

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

NAN_GETTER(WebWorker::OnMessageGet)
{
    NanScope();
    WebWorker* webWorker = ObjectWrap::Unwrap<WebWorker>(args.This());
    Local<Function> onMessage = NanPersistentToLocal(webWorker->_OnMessage);
    NanReturnValue(onMessage);
}

NAN_SETTER(WebWorker::OnMessageSet)
{
    NanScope();
    WebWorker* webWorker = ObjectWrap::Unwrap<WebWorker>(args.This());
    Local<Function> onMessage = value.As<Function>();
    NanAssignPersistent(Function, webWorker->_OnMessage, onMessage);
}

NAN_METHOD(WebWorker::PostMessage)
{
    NanScope();
    NanReturnValue(args.This());
}

NAN_METHOD(WebWorker::AddEventListener)
{
    NanScope();
    NanReturnValue(args.This());
}

NAN_METHOD(WebWorker::Terminate)
{
    NanScope();
    NanReturnValue(Undefined());
}