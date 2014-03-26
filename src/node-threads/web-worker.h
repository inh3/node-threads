#ifndef _WEB_WORKER_H_
#define _WEB_WORKER_H_

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
using namespace v8;
using namespace node;

#include "nan.h"

class WebWorker : public ObjectWrap
{
    public:

        // constructor 'new'
        static NAN_METHOD(New);

        // provide public access to Node's ObjectWrap
        // Ref() and Unref() functions for use by the factory
        virtual void Ref() { ObjectWrap::Ref(); }
        virtual void Unref() { ObjectWrap::Unref(); }

    private:

        explicit WebWorker();
        virtual ~WebWorker();
};

#endif /* _WEB_WORKER_H_ */