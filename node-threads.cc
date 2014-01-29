// v8 and node
#include <v8.h>
#include <node.h>
using namespace v8;

// node threads object
#include "node-threads-object.h"

void init(Handle<Object> exports, Handle<Object> module)
{
    NodeThreads::Init(exports, module);
}

NODE_MODULE(node_threads, init);