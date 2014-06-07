#define _WEB_WORK_ITEM_CC_

#include "web-work-item.h"

// custom
#include "thread.h"
#include "web-worker.h"
#include "json.h"
#include "error-handling.h"
#include "utilities.h"

#include "nan-extra.h"

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
    NanAssignPersistent(_WorkOptions, workOptions);
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
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

    NanScope();

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

    NanScope();

    // get reference to web worker
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

    CreateWorkerContext();
    StoreHiddenReference();

    #if NODE_VERSION_AT_LEAST(0, 11, 13)
    Handle<UnboundScript> workerScript = NanNew<NanUnboundScript>(NanNew<String>(_WorkerScript));
    #else
    Handle<Script> workerScript = NanNew<Script>(NanNew<String>(_WorkerScript));
    #endif
    NanRunScript(workerScript);

    //Utilities::PrintObjectProperties(NanGetCurrentContext()->Global());

    NanAssignPersistent(
        webWorker->_MessageFunction,
        contextObject->Get(NanNew<String>("onmessage")).As<Function>());
}

void WebWorkItem::ExecuteWorkerScript()
{
    printf("WebWorkItem::ExecuteWorkerScript\n");

    StoreHiddenReference();

    Isolate* isolate = Isolate::GetCurrent();

    // get reference to web worker
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);
    WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

    NanScope();

    Local<Function> messageFunction = NanNew(webWorker->_MessageFunction);

    TryCatch tryCatch;

    Handle<Value> eventHandle = JsonUtility::Parse(_EventObject);

    Handle<Value> messageArgs[] = { eventHandle };
    messageFunction.As<Function>()->Call(
            NanGetCurrentContext()->Global(), 1, messageArgs);

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
    NanScope();

    Handle<Function> postMessage = NanNew<FunctionTemplate>(
        WebWorkItem::PostMessage)->GetFunction();
    Handle<Function> addEventListener = NanNew<FunctionTemplate>(
        WebWorkItem::AddEventListener)->GetFunction();
    Handle<Function> close = NanNew<FunctionTemplate>(
        WebWorkItem::Close)->GetFunction();

    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    
    contextObject->Set(NanNew<String>("self"), contextObject);
    contextObject->Set(NanNew<String>("onmessage"), NanUndefined());
    contextObject->Set(NanNew<String>("postMessage"), postMessage);
    contextObject->Set(NanNew<String>("addEventListener"), addEventListener);
    contextObject->Set(NanNew<String>("close"), close);
}

void WebWorkItem::StoreHiddenReference()
{
    NanScope();

    Handle<Object> contextObject = NanGetCurrentContext()->Global();

    // store reference to this web work item
    Handle<ObjectTemplate> objectTemplate = NanNew<ObjectTemplate>();
    objectTemplate->SetInternalFieldCount(1);
    Local<Object> webWorkItemObject = objectTemplate->NewInstance();
    webWorkItemObject->Set(NanNew<String>("Test"), NanNew<Number>(10));
    NanSetInternalFieldPointer(webWorkItemObject, 0, (void*)this);

    // store the reference object as a hidden field
    contextObject->SetHiddenValue(NanNew<String>("WebWorkItem"),
        webWorkItemObject);
}

// NODE METHODS ----------------------------------------------------------------

NAN_METHOD(WebWorkItem::AddEventListener)
{
    Isolate* isolate = Isolate::GetCurrent();

    NanScope();

    if(args.Length() == 2 && args[1]->IsFunction())
    {
        // get reference to web worker
        thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);
        WebWorker* webWorker = (WebWorker*)threadContext->nodeThreads;

        NanAssignPersistent(
            webWorker->_MessageFunction,
            args[1].As<Function>());
    }

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorkItem::PostMessage)
{
    NanScope();

    Handle<Object> contextObject = NanGetCurrentContext()->Global();
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
        webWorkItem->_WorkResult = JsonUtility::Stringify(NanNull());
    }

    NanReturnValue(args.This());
}

NAN_METHOD(WebWorkItem::Close)
{
    NanScope();

    // http://bespin.cz/~ondras/html/classv8_1_1V8.html
    V8::TerminateExecution(Isolate::GetCurrent());

    NanReturnValue(NanUndefined());
}
