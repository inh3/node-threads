#ifndef _THREAD_ISOLATE_H_
#define _THREAD_ISOLATE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

class ThreadIsolate
{
    public:
        
        // global context (per thread)
        static void     InitializeGlobalContext(Isolate* isolate, Handle<Object> globalContext);
};

#endif /* _THREAD_ISOLATE_H_ */