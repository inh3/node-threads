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

    _EventObject = eventObject;
    _WorkerScript = workerScript;
    
    Handle<Object> workOptions = Object::New();
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
    printf("Event Object:\n%s\n", _EventObject);

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

    Handle<Script> workerScript = Script::New(String::New(_WorkerScript));
        workerScript->Run();

#if (NODE_MODULE_VERSION > 0x000B)
    webWorker->_MessageFunction.Reset(
        Isolate::GetCurrent(),
        contextObject->Get(String::NewSymbol("onmessage")).As<Function>());
#else
    webWorker->_MessageFunction = Persistent<Function>::New(
        contextObject->Get(String::NewSymbol("onmessage")).As<Function>());
#endif
}

void WebWorkItem::ExecuteWorkerScript()
{
    printf("WebWorkItem::ExecuteWorkerScript\n");

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