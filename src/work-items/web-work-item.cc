#define _WEB_WORK_ITEM_CC_

#include "web-work-item.h"

// custom
#include "utilities.h"

WebWorkItem::WebWorkItem(
    Handle<Object> webWorker,
    char* eventObject,
    char* workerScript)
    : WorkItem(webWorker)
{
    printf("WebWorkItem::WebWorkItem\n");

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
}

void WebWorkItem::InstanceWorkFunction(Handle<Object> contextObject)
{
#if (NODE_MODULE_VERSION > 0x000B)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    printf("Worker Script:\n%s\n", _WorkerScript);
    printf("Event Object:\n%s\n", _EventObject);

    if(_WorkerScript != NULL)
    {
        Handle<Script> workerScript = Script::New(String::New(_WorkerScript));
        workerScript->Run();
        Utilities::PrintObjectProperties(contextObject);
    }
}

void WebWorkItem::InstanceWorkCallback()
{

}