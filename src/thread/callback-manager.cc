#define _CALLBACK_MANAGER_CC_

#include "callback-manager.h"

// C
//#include <stdio.h>
#include <stdlib.h>

// public instance "constructor"
CallbackManager& CallbackManager::GetInstance()
{
    // lazy instantiation of class instance
    static CallbackManager classInstance;
    return classInstance;
}

CallbackManager::CallbackManager()
{
    callbackQueue = new queue<WorkItem*>();
    SyncCreateMutex(&(this->queueMutex), 0);
}

CallbackManager::~CallbackManager()
{
    SyncLockMutex(&(this->queueMutex));

    while(!this->callbackQueue->empty())
    {
        WorkItem* workItem = this->callbackQueue->front();
        this->callbackQueue->pop();
        delete workItem;
    }
    delete this->callbackQueue;
    
    SyncUnlockMutex(&(this->queueMutex));
    SyncDestroyMutex(&(this->queueMutex));
}

void CallbackManager::AddWorkItem(WorkItem* workItem)
{
    SyncLockMutex(&(this->queueMutex));
    this->callbackQueue->push(workItem);
    SyncUnlockMutex(&(this->queueMutex));
}

WorkItem* CallbackManager::GetWorkItem()
{
    WorkItem* workItem = 0;

    SyncLockMutex(&(this->queueMutex));

    if(!(this->callbackQueue->empty()))
    {
        workItem = this->callbackQueue->front();
        this->callbackQueue->pop();
    }

    SyncUnlockMutex(&(this->queueMutex));

    return workItem;
}