#ifndef _REQUIRE_H_
#define _REQUIRE_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

class Require
{
    public:
        
        static NAN_METHOD(RequireMethod);

    private:

        // ensure default constructor can't get called
        Require();

        // ensure copy constructor methods can't be called
        Require(Require const&);
        void operator=(Require const&);
};

#endif /* _REQUIRE_H_ */