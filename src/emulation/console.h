#ifndef _CONSOLE_H_
#define _CONSOLE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

class Console
{
    public:

        // should be called once per isolate
        static Handle<Object> GetIsolateConsole();

    private:

        static NAN_METHOD(Log);

        // ensure default constructor can't get called
        Console();

        // ensure copy constructor methods can't be called
        Console(Console const&);
        void operator=(Console const&);
};

#endif /* _CONSOLE_H_ */