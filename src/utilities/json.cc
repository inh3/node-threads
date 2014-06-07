#define _JSON_UTILITY_CC_

#include "json.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// threadpool
#include "thread.h"

// custom source
#include "utilities.h"

char* JsonUtility::Stringify(Handle<Value> valueHandle)
{
    NanScope();

    // get reference to JSON object
    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    Handle<Object> jsonObject = contextObject->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> stringifyFunc = jsonObject->Get(NanNew<String>("stringify")).As<Function>();

    // execute stringify
    Handle<Value> stringifyResult = stringifyFunc->Call(jsonObject, 1, &(valueHandle));

    // only process if the result is valid
    char* returnString = NULL;
    if(stringifyResult->IsUndefined() == false)
    {
        // get reference and length of stringified result
        String::Utf8Value utf8ObjectString(Handle<String>::Cast(stringifyResult));
        const char* utf8String = Utilities::ToCString(utf8ObjectString);
        uint32_t utf8StringLength = strlen(utf8String);
        
        // copy the stringified result
        returnString = (char *)malloc(utf8StringLength + 1);
        memset(returnString, 0, utf8StringLength + 1);
        memcpy(returnString, *utf8ObjectString, utf8StringLength);
    }

    return returnString;
}

Handle<Value> JsonUtility::Parse(char* objectString)
{
    NanEscapableScope();

    // short circuit if bad object
    if(objectString == NULL)
    {
        return NanEscapeScope(NanUndefined());
    }

    // get reference to JSON object
    Handle<Object> contextObject = NanGetCurrentContext()->Global();
    Handle<Object> jsonObject = contextObject->Get(NanNew<String>("JSON"))->ToObject();
    Handle<Function> parseFunc = jsonObject->Get(NanNew<String>("parse")).As<Function>();

    // execute parse
    Handle<Value> jsonString = NanNew<String>(objectString);
    Local<Value> valueHandle = parseFunc->Call(jsonObject, 1, &jsonString);

    return NanEscapeScope(valueHandle);
}
