#define _UTILITIES_CC_

#include "utilities.h"

// C
#include <stdio.h>

void Utilities::PrintObjectProperties(Handle<Object> objectHandle)
{
    Local<Array> propertyKeys = (*objectHandle)->GetPropertyNames();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Handle<v8::String> keyString = propertyKeys->Get(keyIndex)->ToString();
        String::AsciiValue propertyName(keyString);
        fprintf(stdout, "[ Property %u ] %s\n", keyIndex, *propertyName);
    }
}