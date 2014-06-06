#ifndef _THREAD_H_
#define _THREAD_H_

// libuv/node
#include <uv.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

// C++
#include <string>
#ifdef __APPLE__
#include <tr1/unordered_map>
using namespace std::tr1;
#else
#include <unordered_map>
#endif
using namespace std;

// custom
#include "persistent-wrap.h"
#include "node-threads-object.h"

typedef unordered_map<string, PersistentWrap*> NativeMap;

typedef struct thread_context_s
{
    // libuv
    uv_async_t*             uv_async_ptr;

    // v8
    Isolate*                thread_isolate;
    Persistent<Context>     isolate_context;

    // native node modules
    NativeMap*              native_modules;

    // thread pool info
    // does not need to be free'd because it is a reference
    // to the object that owns the thread pool
    NodeThreads*            nodeThreads;

} thread_context_t;

class Thread
{
    public:
        
        static void*        ThreadInit(void* initData);
        static void         ThreadPostInit(void* threadContext);
        static void         ThreadDestroy(void* threadContext);

        static void         AsyncCallback(uv_async_t* handle, int status);

    private:

        // libuv
        static void         AsyncCloseCallback(uv_handle_t* handle);
};

#endif /* _THREAD_H_ */
