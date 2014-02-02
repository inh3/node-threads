#ifndef _NODE_THREADS_FACTORY_H_
#define _NODE_THREADS_FACTORY_H_

#include <v8.h>
#include <node.h>
using namespace v8;
using namespace node;

#include "nan.h"

class NodeThreadsFactory
{
    public:
        
        static void Init(Handle<Object> exports, Handle<Object> module);
        static NAN_METHOD(NewInstance);
};

#endif /* _NODE_THREADS_FACTORY_H_ */