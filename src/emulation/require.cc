#define _REQUIRE_CC_

#include "require.h"

// C
#include <stdio.h>

// custom
#include "thread-isolate.h"
#include "persistent-wrap.h"
#include "utilities.h"

void Require::InitializePerIsolate(const FileInfo& nativeModuleFile)
{
// get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif

    // store reference to node util object
    FileInfo& nativeModuleFileRef = const_cast<FileInfo&>(nativeModuleFile);
    Handle<Value> scriptResult = Utilities::CompileScriptSource(
        String::New(nativeModuleFileRef.FileContents()));
    NanAssignPersistent(Function,
        threadContext->native_support,
        scriptResult.As<Function>());

    // load supported native modules
    LoadNativeModules();
}

void Require::LoadNativeModules()
{
    FileInfo utilFile("./src/js/util.js");
    LoadNativeModule(String::New("util"), &utilFile);

    FileInfo pathFile("./src/js/path.js");
    LoadNativeModule(String::New("path"), &pathFile);
}

void Require::LoadNativeModule(Handle<String> moduleName, FileInfo* nativeFileInfo)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

    // Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
    Handle<Context> moduleContext = Context::New(isolate);
    Handle<Context> isolateContext = isolate->GetCurrentContext();
#else
    HandleScope scope;
    Handle<Context> moduleContext = Context::New();
    Handle<Context> isolateContext = Context::GetCurrent();
#endif

    // set the security token to access calling context properties within new context
    moduleContext->SetSecurityToken(isolateContext->GetSecurityToken());

    // enter module context scope
    Context::Scope moduleScope(moduleContext);

    // get reference to current context's object
    Handle<Object> contextObject = moduleContext->Global();

    // clone the calling context properties into this context
    ThreadIsolate::CloneGlobalContext(isolateContext->Global(), contextObject);

    // create the module context
    ThreadIsolate::CreateModuleContext(contextObject, nativeFileInfo);

    // run the script to get the module object
    Handle<Script> moduleScript = Script::New(
        String::New(nativeFileInfo->FileContents()),
        moduleName);
    moduleScript->Run();
    Handle<Object> moduleObject = contextObject->Get(String::New("module"))->ToObject();

    // create object template in order to use object wrap
    Handle<ObjectTemplate> objectTemplate = ObjectTemplate::New();
    objectTemplate->SetInternalFieldCount(1);
    Local<Object> moduleInstance = objectTemplate->NewInstance();

    // copy the module object to module instance
    Utilities::CopyObject(
        moduleInstance,
        moduleObject->Get(String::New("exports"))->ToObject());

    // wrap the object so it can be persisted
    PersistentWrap* moduleWrap = new PersistentWrap();
    moduleWrap->Wrap(moduleInstance);
    moduleWrap->Ref();

    // add to the thread context for later lookup
    String::Utf8Value moduleNameStr(moduleName);
    threadContext->native_modules->insert(make_pair(*moduleNameStr, moduleWrap));
}

// node -----------------------------------------------------------------------

NAN_METHOD(Require::RequireMethod)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)isolate->GetData();

#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(isolate);
    Local<Function> nativeSupportFunc;
    nativeSupportFunc = Local<Function>::New(isolate, threadContext->native_support);
#else
    HandleScope scope;
    Local<Function> nativeSupportFunc;
    nativeSupportFunc = Local<Function>::New(threadContext->native_support);
#endif

    Local<Object> exports;

    Local<Value> moduleArg[] = { args[0] };
    Handle<Value> isSupported = nativeSupportFunc->Call(
        args.This(),
        1,
        moduleArg);
    bool isNativeModule = isSupported->ToBoolean()->Value();

    if(isNativeModule == true)
    {
        String::Utf8Value moduleName(args[0]->ToString());
        // attempt to find the node thread instance by its name
        NativeMap::const_iterator nativeModuleItr =
            threadContext->native_modules->find(*moduleName);

#if (NODE_MODULE_VERSION > 0x000B)
        exports = NanObjectWrapHandle(nativeModuleItr->second);
#else
        exports = Local<Object>::New(
            NanObjectWrapHandle(nativeModuleItr->second));
#endif
    }

    NanReturnValue(exports);
}