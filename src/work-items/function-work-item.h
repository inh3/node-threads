#ifndef _FUNCTION_WORK_ITEM_H_
#define _FUNCTION_WORK_ITEM_H_

// custom
#include "work-item.h"

class FunctionWorkItem : public WorkItem
{
    public:

        FunctionWorkItem(
            const char* functionString,
            Handle<Value> functionParam,
            Handle<Function> callbackFunction,
            Handle<Object> workOptions,
            Handle<Object> calleeObject,
            Handle<Object> nodeThreads);

        virtual ~FunctionWorkItem();

        virtual void    InstanceWorkFunction(Handle<Object> contextObject);
        virtual void    InstanceWorkCallback();

        virtual void    AsyncCallback(
            Handle<Value> errorHandle,
            Handle<Value> infoHandle,
            Handle<Value> resultHandle);

        Persistent<Function>    _CallbackFunction;

    private:

        // ensure default constructor can't get called
        FunctionWorkItem();

        // declare private copy constructor methods to ensure they can't be called
        FunctionWorkItem(FunctionWorkItem const&);
        void operator=(FunctionWorkItem const&);

        void ProcessWorkOptions(Handle<Object> workOptions);

        char* _FunctionString;
        char* _FunctionParam;
};

#endif /* _FUNCTION_WORK_ITEM_H_ */