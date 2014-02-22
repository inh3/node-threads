#ifndef _FUNCTION_WORK_ITEM_H_
#define _FUNCTION_WORK_ITEM_H_

// custom
#include "work-item.h"

class FunctionWorkItem : public WorkItem
{
    public:

        FunctionWorkItem(const char* functionString);
        virtual ~FunctionWorkItem();

        virtual void*   InstanceWorkFunction(const Isolate* isolate);
        virtual void    InstanceWorkCallback();

    private:

        // ensure default constructor can't get called
        FunctionWorkItem();

        // declare private copy constructor methods to ensure they can't be called
        FunctionWorkItem(FunctionWorkItem const&);
        void operator=(FunctionWorkItem const&);

        char* _FunctionString;
};

#endif /* _FUNCTION_WORK_ITEM_H_ */