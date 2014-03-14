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
#include "utilities.h"

FunctionWorkItem::FunctionWorkItem(
    const char* functionString,
    Handle<Function> callbackFunction,
    Handle<Object> workOptions) : WorkItem(callbackFunction, workOptions)
{
    printf("FunctionWorkItem::FunctionWorkItem\n");
    
    size_t fStrLen = strlen(functionString);

    // add 3 extra slots for '(' + function + ')' + '\0'
    _FunctionString = (char*)malloc(fStrLen + 3);
    memset(_FunctionString, 0, fStrLen + 3);

    _FunctionString[0] = '(';
    memcpy(_FunctionString + 1, functionString, fStrLen);
    _FunctionString[fStrLen + 1] = ')';
}

FunctionWorkItem::~FunctionWorkItem()
{
    static int x = 0;
    printf("FunctionWorkItem::~FunctionWorkItem - %u\n", ++x);
    free(_FunctionString);
}

void FunctionWorkItem::InstanceWorkFunction()
{
    //printf("[ FunctionWorkItem::InstanceWorkFunction ]\n");
    
    Handle<Function> functionToExecute;
    Handle<Value> scriptResult = Utilities::CompileScriptSource(String::New(_FunctionString));

    if(scriptResult->IsFunction())
    {
        functionToExecute = scriptResult.As<Function>();
    }
    else if(scriptResult->IsObject() && 
        !(scriptResult.As<Object>()->GetHiddenValue(String::New("exception")).IsEmpty()))
    {
        printf("[ FunctionWorkItem::InstanceWorkFunction ] - Error\n");
        //args[1].As<Function>()->Call(args.This(), 1, &(scriptResult));
    }

    if(!functionToExecute.IsEmpty())
    {
        Handle<Value> workResult = functionToExecute->Call(Context::GetCurrent()->Global(), 0, NULL);
        _WorkResult = JsonUtility::Stringify(workResult);
    }
}

void FunctionWorkItem::InstanceWorkCallback()
{
    //printf("FunctionWorkItem::InstanceWorkCallback\n");
}