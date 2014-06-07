#define _WEB_WORK_ITEM_CC_

#include "web-work-item.h"

// custom
#include "thread.h"
#include "web-worker.h"
#include "json.h"
#include "error-handling.h"
#include "utilities.h"

WebWorkItem::WebWorkItem(
    Handle<Object> webWorker,
    char* eventObject,
    char* workerScript)
    : WorkItem(webWorker)
{
    //printf("WebWorkItem::WebWorkItem\n");

    _AsyncShouldProcess = false;

    _EventObject = eventObject;
    _WorkerScript = workerScript;
    
    Handle<Object> workOptions = NanNew<Object>();
#if (NODE_MODULE_VERSION > 0x000B)
    _WorkOptions.Reset(Isolate::GetCurrent(), workOptions);
#else
    _WorkOptions = Persistent<Object>::New(workOptions);
#endif
}

WebWorkItem::~WebWorkItem()
{
    static int x = 0;
    printf("WebWorkItem::~WebWorkItem - %u\n", ++x);

    free(_EventObject);
    free(_WorkerScript);
}

void WebWorkItem::InstanceWorkFunction(Handle<Object> contextObject)
{
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to web worker
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    //printf("Worker Script:\n%s\n", _WorkerScript);
    //printf("Event Object:\n%s\n", _EventObject);

    if(_WorkerScript != NULL)
    {
        ProcessWorkerScript(contextObject);
    }
    else if((_EventObject != NULL) && !(webWorker->_MessageFunction.IsEmpty()))
    {
        ExecuteWorkerScript();
    }
}

void WebWorkItem::InstanceWorkCallback()
{
}

void WebWorkItem::ProcessWorkerScript(Handle<Object> contextObject)
{
    Isolate* isolate = Isolate::GetCurrent();

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    // get reference to web worker
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

    CreateWorkerContext();
    StoreHiddenReference();

    Handle<Script> workerScript = Script::New(NanNew<String>(_WorkerScript));
        workerScript->Run();

    //Utilities::PrintObjectProperties(Context::GetCurrent()->Global());

#if (NODE_MODULE_VERSION > 0x000B)
    webWorker->_MessageFunction.Reset(
        Isolate::GetCurrent(),
        contextObject->Get(NanNew<String>("onmessage")).As<Function>());
#else
    webWorker->_MessageFunction = Persistent<Function>::New(
        contextObject->Get(NanNew<String>("onmessage")).As<Function>());
#endif
}

void WebWorkItem::ExecuteWorkerScript()
{
    printf("WebWorkItem::ExecuteWorkerScript\n");

    StoreHiddenReference();

    Isolate* isolate = Isolate::GetCurrent();

    // get reference to web worker
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);

    Local<Function> messageFunction = Local<Function>::New(
        Isolate::GetCurrent(),
        webWorker->_MessageFunction);
#else
    HandleScope scope;

    Local<Function> messageFunction = Local<Function>::New(
        webWorker->_MessageFunction);
#endif

    TryCatch tryCatch;

    Handle<Value> eventHandle = JsonUtility::Parse(_EventObject);

    Handle<Value> messageArgs[] = { eventHandle };
    messageFunction.As<Function>()->Call(
            Context::GetCurrent()->Global(), 1, messageArgs);

    if(tryCatch.HasCaught())
    {
        Handle<Object> exceptionObject = ErrorHandling::HandleException(&tryCatch);
        _Exception = JsonUtility::Stringify(exceptionObject);
    }
    else
    {
        printf("\n\n*** Executed Successfully!\n\n");
    }
}

void WebWorkItem::CreateWorkerContext()
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    Handle<Function> postMessage = NanNew<FunctionTemplate>(
        WebWorkItem::PostMessage)->GetFunction();
    Handle<Function> addEventListener = NanNew<FunctionTemplate>(
        WebWorkItem::AddEventListener)->GetFunction();
    Handle<Function> close = NanNew<FunctionTemplate>(
        WebWorkItem::Close)->GetFunction();

    Handle<Object> contextObject = Context::GetCurrent()->Global();
    
    contextObject->Set(NanNew<String>("self"), contextObject);
    contextObject->Set(NanNew<String>("onmessage"), Undefined());
    contextObject->Set(NanNew<String>("postMessage"), postMessage);
    contextObject->Set(NanNew<String>("addEventListener"), addEventListener);
    contextObject->Set(NanNew<String>("close"), close);
}

void WebWorkItem::StoreHiddenReference()
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    Handle<Object> contextObject = Context::GetCurrent()->Global();

    // store reference to this web work item
    Handle<ObjectTemplate> objectTemplate = ObjectTemplate::New();
    objectTemplate->SetInternalFieldCount(1);
    Local<Object> webWorkItemObject = objectTemplate->NewInstance();
    webWorkItemObject->Set(NanNew<String>("Test"), Number::New(10));
    NanSetInternalFieldPointer(webWorkItemObject, 0, (void*)this);

    // store the reference object as a hidden field
    contextObject->SetHiddenValue(NanNew<String>("WebWorkItem"),
        webWorkItemObject);
}

// NODE METHODS ----------------------------------------------------------------

NAN_METHOD(WebWorkItem::AddEventListener)
{
    Isolate* isolate = Isolate::GetCurrent();

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    if(args.Length() == 2 && args[1]->IsFunction())
    {
        // get reference to web worker
        thread_context_t *threadContext = (thread_context_t*)isolate->GetData();
        WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

        #if (NODE_MODULE_VERSION > 0x000B)
            webWorker->_MessageFunction.Reset(
                Isolate::GetCurrent(),
                args[1].As<Function>());
        #else
            webWorker->_MessageFunction = Persistent<Function>::New(
                args[1].As<Function>());
        #endif
    }

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorkItem::PostMessage)
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    Handle<Object> contextObject = Context::GetCurrent()->Global();
    Handle<Object> workItemObject = contextObject->GetHiddenValue(
        NanNew<String>("WebWorkItem")).As<Object>();
    WebWorkItem* webWorkItem = (WebWorkItem*)NanGetInternalFieldPointer(workItemObject, 0);
    webWorkItem->_AsyncShouldProcess = true;

    if(args.Length() == 1)
    {
        webWorkItem->_WorkResult = JsonUtility::Stringify(args[0]);
    }
    else
    {
        webWorkItem->_WorkResult = JsonUtility::Stringify(Null());
    }

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorkItem::Close)
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    // http://bespin.cz/~ondras/html/classv8_1_1V8.html
    V8::TerminateExecution(Isolate::GetCurrent());

    NanReturnValue(Undefined());
}
