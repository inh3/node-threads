#define _THREAD_CC_

#include "thread.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// libuv/node
#include <uv.h>
#include <node.h>
#include <v8.h>
using namespace v8;

typedef struct thread_context_s
{
    // libuv
    uv_async_t*             uv_async_ptr;

    // v8
    Isolate*                thread_isolate;
    Persistent<Context>     thread_context;

} thread_context_t;

void* Thread::ThreadInit()
{
    // allocate memory for thread context
    thread_context_t* threadContext = (thread_context_t*)malloc(sizeof(thread_context_t));
    memset(threadContext, 0, sizeof(thread_context_t));

    // create and initialize async watcher
    threadContext->uv_async_ptr = (uv_async_t*)malloc(sizeof(uv_async_t));
    memset(threadContext->uv_async_ptr, 0, sizeof(uv_async_t));
    uv_async_init(uv_default_loop(), threadContext->uv_async_ptr, NULL);
    //threadContext->uvAsync->close_cb = Thread::uvCloseCallback;

    // create thread isolate
    threadContext->thread_isolate = Isolate::New();

    return (void*)threadContext;
}

void Thread::ThreadPostInit(void* threadContext)
{
    
}

void Thread::ThreadDestroy(void* threadContext)
{
    // thread context
    thread_context_t* thisContext = (thread_context_t*)threadContext;

    // dispose of the isolate
    thisContext->thread_isolate->Dispose();

    //uv_unref((uv_handle_t*)thisContext->uvAsync);
    uv_close((uv_handle_t*)thisContext->uv_async_ptr, NULL);

    // release the thread context memory
    free(thisContext);
}