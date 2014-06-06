#ifndef _NODE_THREADS_FACTORY_H_
#define _NODE_THREADS_FACTORY_H_

//using namespace node;
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

// node-threads
#include "utilities.h"
#include "node-threads-object.h"

typedef unordered_map<string, NodeThreads*> NodeThreadMap;

class NodeThreadsFactory
{
    public:
        
        static void Init();
        static NAN_METHOD(CreateInstance);
        static NAN_METHOD(DestroyInstance);

    private:

        static NodeThreadMap _NodeThreadMap;
};

#endif /* _NODE_THREADS_FACTORY_H_ */
