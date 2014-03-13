#ifndef _CALLBACK_MANAGER_H_
#define _CALLBACK_MANAGER_H_

// C++
#include <queue>
using namespace std;

// custom source
#include "work-item.h"
#include "synchronize.h"

class CallbackManager
{
    public:
        
        // singleton instance of class
        static CallbackManager&     GetInstance();

        // destructor
        virtual                     ~CallbackManager();

        // load work item into queue
        void                        AddWorkItem(WorkItem* workItem);

        // get work item from queue and remove it
        WorkItem*                   GetWorkItem();

    protected:

        // ensure default constructor can't get called
        CallbackManager();

        // declare private copy constructor methods
        // to ensure they can't be called
        CallbackManager(CallbackManager const&);
        void operator=(CallbackManager const&);

    private:

        queue<WorkItem*>            *callbackQueue;
        THREAD_MUTEX                queueMutex;
};

#endif /* _CALLBACK_MANAGER_H_ */