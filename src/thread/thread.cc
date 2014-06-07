#define _THREAD_CC_

#include "thread.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom
#include "web-worker.h"
#include "thread-isolate.h"
#include "callback-manager.h"
#include "work-item.h"
#include "json.h"
#include "utilities.h"

#include "nan-extra.h"

// callback manager
static CallbackManager* callbackManager = &(CallbackManager::GetInstance());

void* Thread::ThreadInit(void* initData)
{
    // allocate memory for thread context
    thread_context_t* threadContext = (thread_context_t*)malloc(sizeof(thread_context_t));
    memset(threadContext, 0, sizeof(thread_context_t));

    // create the native module map
    threadContext->native_modules = new NativeMap();

    // create and initialize async watcher
    threadContext->uv_async_ptr = (uv_async_t*)malloc(sizeof(uv_async_t));
    memset(threadContext->uv_async_ptr, 0, sizeof(uv_async_t));
    uv_async_init(
        uv_default_loop(),
        threadContext->uv_async_ptr,
        Thread::AsyncCallback);

    // create thread isolate
    threadContext->thread_isolate = Isolate::New();

    // store reference to the node threads object
    threadContext->nodeThreads = (NodeThreads*)initData;

    // store reference to thread context within isolate
    IsolateSetData(threadContext->thread_isolate, threadContext);

    return (void*)threadContext;
}

void Thread::ThreadPostInit(void* threadContext)
{
    // get reference to the thread context
    thread_context_t* thisContext = (thread_context_t*)threadContext;

    // get reference to thread isolate
    Isolate* isolate = thisContext->thread_isolate;
    {
        // lock the isolate
        Locker isolateLocker(isolate);

        // enter the isolate
        isolate->Enter();

        ThreadIsolate::InitializeGlobalContext();
    }

    // leave the isolate
    isolate->Exit();
}

void Thread::ThreadDestroy(void* threadContext)
{
    // thread context
    thread_context_t* thisContext = (thread_context_t*)threadContext;

    // get reference to thread isolate
    Isolate* isolate = thisContext->thread_isolate;
    {
        // lock the isolate
        Locker myLocker(isolate);

        // enter the isolate
        isolate->Enter();

        // clean-up the native modules
        for(NativeMap::iterator it = thisContext->native_modules->begin(); it != thisContext->native_modules->end(); ++it)
        {
            PersistentWrap* pWrap = it->second;
            pWrap->Unref();
#if (NODE_MODULE_VERSION > 0x000B)
            NanDisposePersistent(pWrap->persistent());
#else
            NanDisposePersistent(pWrap->handle_);
#endif
            delete pWrap;
        }
        thisContext->native_modules->clear();

        // dispose web worker message function
        NodeThreads* nodeThreads = thisContext->nodeThreads;
        if(nodeThreads->IsWebWorker())
        {
            WebWorker* webWorker = (WebWorker*)nodeThreads;
            NanDisposePersistent(webWorker->_MessageFunction);
        }

        // dispose of js context
        NanDisposePersistent(thisContext->isolate_context);
    }

    // exit the isolate
    isolate->Exit();

    // dispose of the isolate
    thisContext->thread_isolate->Dispose();

    //uv_unref((uv_handle_t*)thisContext->uvAsync);
    uv_close((uv_handle_t*)thisContext->uv_async_ptr, Thread::AsyncCloseCallback);

    // release the native modules
    delete thisContext->native_modules;

    // release the thread context memory
    free(thisContext);
}

#if NODE_VERSION_AT_LEAST(0, 11, 13)
void Thread::AsyncCallback(uv_async_t* handle)
#else
void Thread::AsyncCallback(uv_async_t* handle, int status)
#endif
{
    NanScope();

    printf("Thread::AsyncCallback\n");

    // process all work items awaiting callback
    WorkItem* workItem = NULL;
    while((workItem = callbackManager->GetWorkItem()) != 0)
    {
        if(workItem->_AsyncShouldProcess == true)
        {
            Local<Object> workOptions = NanNew(workItem->_WorkOptions);

            Handle<Value> exceptionHandle = JsonUtility::Parse(workItem->_Exception);
            Handle<Value> workResultHandle = JsonUtility::Parse(workItem->_WorkResult);

            workOptions->Set(
                NanNew<String>("thread"), NanNew<Number>(workItem->_ThreadId));
            workOptions->Set(
                NanNew<String>("threadpool"), NanNew<String>(workItem->_ThreadPoolKey.c_str()));

            workItem->AsyncCallback(
                (exceptionHandle == NanUndefined() ? (Handle<Value>)NanNull() : exceptionHandle),
                workOptions,
                workResultHandle);
        }

        delete workItem;
    }
}

void Thread::AsyncCloseCallback(uv_handle_t* handle)
{
    // cleanup the handle
    free(handle);
}
