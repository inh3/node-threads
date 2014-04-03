#ifndef _WEB_WORK_ITEM_H_
#define _WEB_WORK_ITEM_H_

// custom
#include "work-item.h"

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

class WebWorkItem : public WorkItem
{
    public:

        WebWorkItem(
            Handle<Object> webWorker,
            char* eventObject);

        virtual ~WebWorkItem();

        virtual void    InstanceWorkFunction(Handle<Object> contextObject);
        virtual void    InstanceWorkCallback();

    private:

        char*             _EventObject;

        // ensure default constructor can't get called
        WebWorkItem();

        // declare private copy constructor methods to ensure they can't be called
        WebWorkItem(WebWorkItem const&);
        void operator=(WebWorkItem const&);
};

#endif /* _WEB_WORK_ITEM_H_ */