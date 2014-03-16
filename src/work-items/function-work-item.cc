#define _FUNCTION_WORK_ITEM_CC_

#include "function-work-item.h"

// node
#include <node.h>
#include <v8.h>
using namespace v8;

#include "nan.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom
#include "json.h"
#include "error-handling.h"
#include "utilities.h"

FunctionWorkItem::FunctionWorkItem(
    const char* functionString,
    Handle<Function> callbackFunction,
    Handle<Object> workOptions,
    Handle<Object> calleeObject)
    : WorkItem(callbackFunction, workOptions, calleeObject)
{
    printf("FunctionWorkItem::FunctionWorkItem\n");
    
    size_t fStrLen = strlen(functionString);

    // add 3 extra slots for '(' + function + ')' + '\0'
    _FunctionString = (char*)malloc(fStrLen + 5);
    memset(_FunctionString, 0, fStrLen + 5);

    _FunctionString[0] = '(';
    memcpy(_FunctionString + 1, functionString, fStrLen);
    _FunctionString[1 + fStrLen] = ')';
    _FunctionString[2 + fStrLen] = '(';
    _FunctionString[3 + fStrLen] = ')';
}

FunctionWorkItem::~FunctionWorkItem()
{
    static int x = 0;
    printf("FunctionWorkItem::~FunctionWorkItem - %u\n", ++x);
    free(_FunctionString);
}

void FunctionWorkItem::InstanceWorkFunction()
{
    printf("[ FunctionWorkItem::InstanceWorkFunction ]\n");

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(Isolate::GetCurrent());
#else
    HandleScope scope;
#endif
    
    TryCatch tryCatch;

    Handle<Script> compiledScript;
    Handle<Value> scriptResult;
    Handle<Object> exceptionObject;

    //String::Utf8Value scriptSource(String::New(_FunctionString));
    //printf("%s\n", *scriptSource);
    compiledScript = Script::Compile(String::New(_FunctionString));

    // check for exception on compile
    if(compiledScript.IsEmpty() || tryCatch.HasCaught())
    {
        exceptionObject = ErrorHandling::HandleException(&tryCatch);
        _Exception = JsonUtility::Stringify(exceptionObject);
    }
    else
    {
        scriptResult = compiledScript->Run();

        // check that running script didn't throw errors
        if(scriptResult.IsEmpty() || tryCatch.HasCaught())
        {
            exceptionObject = ErrorHandling::HandleException(&tryCatch);
            _Exception = JsonUtility::Stringify(exceptionObject);
        }
        else
        {
            _WorkResult = JsonUtility::Stringify(scriptResult);
        }
    }
}

void FunctionWorkItem::InstanceWorkCallback()
{
    //printf("FunctionWorkItem::InstanceWorkCallback\n");
}