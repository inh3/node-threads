#ifndef _NODE_THREADS_OBJECT_H_
#define _NODE_THREADS_OBJECT_H_

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
using namespace v8;
using namespace node;

#include "nan.h"

class NodeThreads : public ObjectWrap
{
    public:
        
        static void Init(Handle<Object> exports, Handle<Object> module);

        static NAN_METHOD(Test);
    
    private:

        explicit NodeThreads();
        ~NodeThreads();

        static NAN_METHOD(New);

        static Persistent<Function> Constructor;
        static Persistent<Function> EventEmitter;
};

#endif /* _NODE_THREADS_OBJECT_H_ */