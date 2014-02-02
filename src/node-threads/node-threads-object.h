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

        static NAN_METHOD(New);
        static NAN_METHOD(Test);

        static Persistent<Function> Constructor;
        static Persistent<Function> EventEmitter;
    
    private:

        explicit NodeThreads();
        ~NodeThreads();
};

#endif /* _NODE_THREADS_OBJECT_H_ */