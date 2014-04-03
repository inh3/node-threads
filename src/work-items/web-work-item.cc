#define _WEB_WORK_ITEM_CC_

#include "web-work-item.h"

WebWorkItem::WebWorkItem(
    Handle<Object> webWorker,
    char* eventObject)
    : WorkItem(webWorker)
{
    printf("WebWorkItem::WebWorkItem\n");

    _EventObject = eventObject;
    
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
}

void WebWorkItem::InstanceWorkCallback()
{

}