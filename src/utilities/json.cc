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

void JsonUtility::Initialize()
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    // get reference to JSON object
    Handle<Object> contextObject = Context::GetCurrent()->Global();
    Handle<Object> jsonObject = contextObject->Get(v8::String::New("JSON"))->ToObject();
    NanAssignPersistent(Object, threadContext->json_object, jsonObject);

    // store reference to stringify
    Handle<Function> stringifyFunc = jsonObject->Get(v8::String::New("stringify")).As<Function>();
    NanAssignPersistent(Function, threadContext->json_stringify, stringifyFunc);

    // store reference to parse
    Handle<Function> parseFunc = jsonObject->Get(v8::String::New("parse")).As<Function>();
    NanAssignPersistent(Function, threadContext->json_parse, parseFunc);
}

char* JsonUtility::Stringify(Handle<Value> valueHandle)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    // get handle to stringify function
    Local<Function> stringifyFunc = NanPersistentToLocal(threadContext->json_stringify);

    // execute stringify
    Local<Object> jsonObject = NanPersistentToLocal(threadContext->json_object);
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
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope handleScope(isolate);
#else
    HandleScope handleScope;
#endif

    // get handle to stringify function
    Local<Function> parseFunc = NanPersistentToLocal(threadContext->json_parse);

    // execute parse
    Handle<Value> jsonString = String::New(objectString);
    Local<Object> jsonObject = NanPersistentToLocal(threadContext->json_object);
    Local<Value> valueHandle = parseFunc->Call(jsonObject, 1, &jsonString);

    return handleScope.Close(valueHandle);
}