#ifndef _WEB_WORK_ITEM_H_
#define _WEB_WORK_ITEM_H_

// custom
#include "work-item.h"

// node
#include <v8.h>
using namespace v8;

#include "nan.h"

class WebWorkItem : public WorkItem
{
    public:

        WebWorkItem(
            Handle<Object> webWorker,
            char* eventObject,
            char* workerScript);

        virtual ~WebWorkItem();

        virtual void    InstanceWorkFunction(Handle<Object> contextObject);
        virtual void    InstanceWorkCallback();

    private:

        char*                   _EventObject;
        char*                   _WorkerScript;

        void CreateWorkerContext();
        void StoreHiddenReference();
        void ProcessWorkerScript(Handle<Object> contextObject);
        void ExecuteWorkerScript();

        // ensure default constructor can't get called
        WebWorkItem();

        // declare private copy constructor methods to ensure they can't be called
        WebWorkItem(WebWorkItem const&);
        void operator=(WebWorkItem const&);

        // constructor 'new'
        static NAN_METHOD(AddEventListener);
        static NAN_METHOD(PostMessage);
        static NAN_METHOD(Close);
};

#endif /* _WEB_WORK_ITEM_H_ */
