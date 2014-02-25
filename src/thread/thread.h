#ifndef _THREAD_H_
#define _THREAD_H_

// libuv/node
#include <uv.h>
#include <node.h>
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

typedef unordered_map<string, PersistentWrap*> NativeMap;

typedef struct thread_context_s
{
    // libuv
    uv_async_t*             uv_async_ptr;

    // v8
    Isolate*                thread_isolate;
    Persistent<Context>     isolate_context;

    // native node modules
    Persistent<Object>      node_util;
    Persistent<Function>    native_support;
    NativeMap*              native_modules;

    // json
    Persistent<Object>      json_object;
    Persistent<Function>    json_stringify;
    Persistent<Function>    json_parse;

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