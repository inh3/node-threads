#ifndef _THREAD_H_
#define _THREAD_H_

// libuv/node
#include <uv.h>
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

typedef struct thread_context_s
{
    // libuv
    uv_async_t*             uv_async_ptr;

    // v8
    Isolate*                thread_isolate;
    Persistent<Context>     isolate_context;

} thread_context_t;

class Thread
{
    public:
        
        static void*        ThreadInit();
        static void         ThreadPostInit(void* threadContext);
        static void         ThreadDestroy(void* threadContext);

    private:

        // libuv
        static void         AsyncCloseCallback(uv_handle_t* handle);
};

#endif /* _THREAD_H_ */