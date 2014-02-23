#define _THREAD_CC_

#include "thread.h"

// node
#include <node_version.h>

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom
#include "thread-isolate.h"

void* Thread::ThreadInit()
{
    // allocate memory for thread context
    thread_context_t* threadContext = (thread_context_t*)malloc(sizeof(thread_context_t));
    memset(threadContext, 0, sizeof(thread_context_t));

    // create and initialize async watcher
    threadContext->uv_async_ptr = (uv_async_t*)malloc(sizeof(uv_async_t));
    memset(threadContext->uv_async_ptr, 0, sizeof(uv_async_t));
    uv_async_init(uv_default_loop(), threadContext->uv_async_ptr, NULL);

    // create thread isolate
    threadContext->thread_isolate = Isolate::New();

    // store reference to thread context within isolate
    threadContext->thread_isolate->SetData((void*)threadContext);

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

        // dispose of node util object
        thisContext->node_util.Dispose();

        // dispose of js context
        thisContext->isolate_context.Dispose();
    }

    // exit the isolate
    isolate->Exit();

    // dispose of the isolate
    thisContext->thread_isolate->Dispose();

    //uv_unref((uv_handle_t*)thisContext->uvAsync);
    uv_close((uv_handle_t*)thisContext->uv_async_ptr, Thread::AsyncCloseCallback);

    // release the thread context memory
    free(thisContext);
}

void Thread::AsyncCloseCallback(uv_handle_t* handle)
{
    // cleanup the handle
    free(handle);
}