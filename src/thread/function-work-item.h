#ifndef _FUNCTION_WORK_ITEM_H_
#define _FUNCTION_WORK_ITEM_H_

// custom
#include "work-item.h"

class FunctionWorkItem : public WorkItem
{
    public:

        FunctionWorkItem();
        virtual ~FunctionWorkItem();

        virtual void*   InstanceWorkFunction();
        virtual void    InstanceWorkCallback();
};

#endif /* _FUNCTION_WORK_ITEM_H_ */