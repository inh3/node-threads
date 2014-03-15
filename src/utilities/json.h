#ifndef _JSON_UTILITY_H_
#define _JSON_UTILITY_H_

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

class JsonUtility
{
    public:

        static char*            Stringify(Handle<Value> v8Handle);
        static Handle<Value>    Parse(char* jsonObject);
};

#endif /* _JSON_UTILITY_H_ */