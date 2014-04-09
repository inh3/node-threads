#define _WEB_WORKER_C_

#include "web-worker.h"

// custom
#include "environment.h"
#include "web-work-item.h"
#include "json.h"
#include "utilities.h"

Persistent<Function> WebWorker::Constructor;

WebWorker::WebWorker(const char* threadPoolKey, bool isFunction)
    : NodeThreads(threadPoolKey, 1, true)
{
    printf("WebWorker::WebWorker\n");

    _IsFunction = isFunction;
}

WebWorker::~WebWorker()
{
    printf("WebWorker::~WebWorker\n");

    _OnMessage.Dispose();
    _OnMessage.Clear();
}

void WebWorker::Init()
{
    // prepare the constructor function template
    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New(WebWorker::New);
    constructorTemplate->InstanceTemplate()->SetAccessor(
        String::NewSymbol("onmessage"),
        WebWorker::OnMessageGet,
        WebWorker::OnMessageSet);
    constructorTemplate->SetClassName(String::NewSymbol("Worker"));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // inherit from event emitter
    Local<Function> inheritsFunction = NanPersistentToLocal(
        Environment::Util)->Get(String::NewSymbol("inherits")).As<Function>();

    Local<Value> inheritArgs[] = { 
        constructorTemplate->GetFunction(),
        NanPersistentToLocal(Environment::EventEmitter)
    };
    inheritsFunction->Call(NanPersistentToLocal(Environment::Module), 2, inheritArgs);
    
    NanAssignPersistent(
        Function,
        WebWorker::Constructor,
        constructorTemplate->GetFunction());
}

void WebWorker::QueueWebWorker(
    char* eventObject,
    char* workerScript,
    Handle<Object> threadPoolObject)
{
    WebWorkItem* webWorkItem = new WebWorkItem(
        threadPoolObject, eventObject, workerScript);
    QueueWorkItem((void*)webWorkItem);
}

char* WebWorker::ProcessFunction(Handle<Function> workerFunction)
{
    NanScope();

    String::Utf8Value funcStr(workerFunction->ToString());
    string funcToExe(*funcStr);
    funcToExe.insert(0, "(");
    funcToExe.append(")()");

    char* returnStr = (char*)malloc(funcToExe.length() + 1);
    memset(returnStr, 0, funcToExe.length() + 1);
    memcpy(returnStr, funcToExe.c_str(), funcToExe.length() + 1);

    return returnStr;
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
        Local<Function> eventEmitter = NanPersistentToLocal(Environment::EventEmitter);
        eventEmitter->Call(args.This(), 0, NULL);

        // get reference to callee object directory
        Handle<Object> calleeObject = NodeThreads::GetCalleeInfo().As<Object>();
        String::Utf8Value dirNameStr(
            calleeObject->Get(String::NewSymbol("__dirname")));

        // determine if this is being created from a file or a function
        bool isFunction = false;
        if(args[0]->IsFunction())
        {
            isFunction = true;
        }
        else
        {

        }

        // create and wrap the class and return the javascript object
        WebWorker* webWorker = new WebWorker("WebWorker", isFunction);
        char* funcStr = webWorker->ProcessFunction(args[0].As<Function>());
        webWorker->Wrap(args.This());

        webWorker->QueueWebWorker(
            NULL,
            funcStr,
            args.This());

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

    // remove existing listener
    Handle<Function> removeAllFunc = args.This()->Get(
            String::NewSymbol("removeAllListeners")).As<Function>();
    Handle<Value> removeAllArgs[] = { String::New("message") };
    removeAllFunc->Call(args.This(), 1, removeAllArgs);

    // add then new listener
    Handle<Function> onFunction = args.This()->Get(
            String::NewSymbol("on")).As<Function>();
    Handle<Value> onArgs[] = { String::New("message"), value };
    onFunction->Call(args.This(), 2, onArgs);
}

NAN_METHOD(WebWorker::PostMessage)
{
    NanScope();

    WebWorker* webWorker = ObjectWrap::Unwrap<WebWorker>(args.This());

    Handle<Object> eventObject = Object::New();

    eventObject->Set(
            String::NewSymbol("data"),
            NanNewLocal<Object>(args[0].As<Object>()));
    char* eventStringified = JsonUtility::Stringify(eventObject);

    webWorker->QueueWebWorker(
        eventStringified,
        NULL,
        args.This());

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorker::AddEventListener)
{
    NanScope();

    if((args.Length() == 2) && args[0]->IsString() && args[1]->IsFunction())
    {
        Handle<Function> onFunction = args.This()->Get(
            String::NewSymbol("on")).As<Function>();

        Handle<Value> onArgs[] = { args[0], args[1] };
        onFunction->Call(args.This(), 2, onArgs);
    }

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorker::Terminate)
{
    NanScope();
    NanReturnValue(Undefined());
}