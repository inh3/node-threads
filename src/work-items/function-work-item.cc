#define _FUNCTION_WORK_ITEM_CC_

#include "function-work-item.h"

// node
#include <v8.h>
using namespace v8;

#include "nan.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom
#include "nt-environment.h"
#include "json.h"
#include "error-handling.h"
#include "utilities.h"

static const char* FunctionPrefix = "(function(){return ";
static const uint32_t PrefixLen = strlen(FunctionPrefix);
static const char* FunctionPostfix = ";})();";
static const uint32_t PostFixLen = strlen(FunctionPostfix);

FunctionWorkItem::FunctionWorkItem(
    const char* functionString,
    Handle<Value> functionParam,
    Handle<Function> callbackFunction,
    Handle<Object> workOptions,
    Handle<Object> calleeObject,
    Handle<Object> nodeThreads)
    : WorkItem(nodeThreads)
{
    printf("FunctionWorkItem::FunctionWorkItem\n");

    _AsyncShouldProcess = true;
    
    size_t fStrLen = strlen(functionString);

    _FunctionParam = JsonUtility::Stringify(functionParam);

    _FunctionString = (char*)malloc(fStrLen + PrefixLen + PostFixLen + 1);
    memset(_FunctionString, 0, fStrLen + PrefixLen + PostFixLen + 1);

    uint32_t stringOffset = 0;
    memcpy(_FunctionString, FunctionPrefix, PrefixLen);
    stringOffset += PrefixLen;
    
    memcpy(_FunctionString + stringOffset, functionString, fStrLen);
    stringOffset += fStrLen;

    memcpy(_FunctionString + stringOffset, FunctionPostfix, PostFixLen);

#if (NODE_MODULE_VERSION > 0x000B)
    _CallbackFunction.Reset(Isolate::GetCurrent(), callbackFunction);
#else
    _CallbackFunction = Persistent<Function>::New(callbackFunction);
#endif

    String::Utf8Value fileNameStr(calleeObject->Get(
        NanNew<String>("__filename")));
    String::Utf8Value dirNameStr(calleeObject->Get(
        NanNew<String>("__dirname")));

    _FileName.assign(*fileNameStr);
    _DirName.assign(*dirNameStr);

    ProcessWorkOptions(workOptions);
}

void FunctionWorkItem::ProcessWorkOptions(Handle<Object> workOptions)
{
    NanScope();

    if(workOptions.IsEmpty())
    {
        workOptions = NanNew<Object>();
    }

    // set context to default node context if not specified
    Handle<Value> contextHandle = workOptions->Get(NanNew<String>("context"));
    if(contextHandle == NanUndefined() || contextHandle.IsEmpty())
    {
        Handle<Object> currentContext = NanGetCurrentContext()->Global();
        workOptions->Set(NanNew<String>("context"), currentContext);
    }

    // set work id to guid if not specified
    Handle<Value> workId = workOptions->Get(NanNew<String>("id"));
    if(workId == Undefined() || workId.IsEmpty())
    {
#if (NODE_MODULE_VERSION > 0x000B)
        Local<Function> guidFunction = Local<Function>::New(
            Isolate::GetCurrent(),
            NTEnvironment::Guid);
#else
        Local<Function> guidFunction = Local<Function>::New(
            NTEnvironment::Guid);
#endif

        Handle<Value> guidHandle = guidFunction->Call(
            Context::GetCurrent()->Global(),
            0,
            NULL);
        workOptions->Set(NanNew<String>("id"), guidHandle);
    }

#if (NODE_MODULE_VERSION > 0x000B)
    _WorkOptions.Reset(Isolate::GetCurrent(), workOptions);
#else
    _WorkOptions = Persistent<Object>::New(workOptions);
#endif
}

FunctionWorkItem::~FunctionWorkItem()
{
    static int x = 0;
    printf("FunctionWorkItem::~FunctionWorkItem - %u\n", ++x);

    _CallbackFunction.Dispose();
    _CallbackFunction.Clear();

    free(_FunctionString);
    free(_FunctionParam);
}

void FunctionWorkItem::InstanceWorkFunction(Handle<Object> contextObject)
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

    // set __filename and __dirname
    contextObject->Set(
        NanNew<String>("__filename"),
        NanNew<String>(_FileName.c_str()));

    contextObject->Set(
        NanNew<String>("__dirname"),
        NanNew<String>(_DirName.c_str()));

    //String::Utf8Value scriptSource(NanNew<String>(_FunctionString));
    //printf("%s\n", *scriptSource);
    compiledScript = Script::Compile(NanNew<String>(_FunctionString));

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
        else if(!scriptResult->IsFunction())
        {
            _WorkResult = JsonUtility::Stringify(Null());
        }
        else
        {
            Handle<Value> argv[1] = { JsonUtility::Parse(_FunctionParam) };
            Handle<Value> functionResult = scriptResult.As<Function>()->Call(
                Context::GetCurrent()->Global(), 1, argv);

            if(tryCatch.HasCaught())
            {
                exceptionObject = ErrorHandling::HandleException(&tryCatch);
                _Exception = JsonUtility::Stringify(exceptionObject);
            }
            else
            {
                _WorkResult = JsonUtility::Stringify(functionResult);
            }
        }
    }
}

void FunctionWorkItem::InstanceWorkCallback()
{
    //_AsyncShouldProcess = true;
}

void FunctionWorkItem::AsyncCallback(
    Handle<Value> errorHandle,
    Handle<Value> infoHandle,
    Handle<Value> resultHandle)
{
    NanScope();

    WorkItem::AsyncCallback(errorHandle, infoHandle, resultHandle);

#if (NODE_MODULE_VERSION > 0x000B)
        Local<Function> callbackFunction = Local<Function>::New(
            Isolate::GetCurrent(),
            _CallbackFunction);
#else
        Local<Function> callbackFunction = Local<Function>::New(
            _CallbackFunction);
#endif

    if(!callbackFunction.IsEmpty() &&
        callbackFunction != Null() &&
        callbackFunction != Undefined())
    {
        //create arguments array
        const unsigned argc = 3;
        Handle<Value> argv[argc] = { 
            // error
            (errorHandle == Undefined() ? (Handle<Value>)Null() : errorHandle),
            // info
            infoHandle,
            // result
            resultHandle
        };

        #if (NODE_MODULE_VERSION <= 0x000B)
            TryCatch tryCatch;
        #endif

            // make callback on node thread
            callbackFunction->Call(
                infoHandle.As<Object>()->Get(NanNew<String>("context")).As<Object>(),
                argc,
                argv);

        #if (NODE_MODULE_VERSION <= 0x000B)
            if(tryCatch.HasCaught())
            {
                tryCatch.ReThrow();
            }
        #endif
    }
}
