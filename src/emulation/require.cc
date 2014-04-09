#define _REQUIRE_CC_

#include "require.h"

// C
#include <stdio.h>

// custom
#include "thread.h"
#include "thread-isolate.h"
#include "persistent-wrap.h"
#include "utilities.h"
#include "nt-environment.h"

void Require::InitializePerIsolate()
{
    string modulePath;

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/util.js");
    FileInfo utilFile(modulePath.c_str());
    LoadNativeModule(String::New("util"), &utilFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/path.js");
    FileInfo pathFile(modulePath.c_str());
    LoadNativeModule(String::New("path"), &pathFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/assert.js");
    FileInfo assertFile(modulePath.c_str());
    LoadNativeModule(String::New("assert"), &assertFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/console.js");
    FileInfo consoleFile(modulePath.c_str());
    LoadNativeModule(String::New("console"), &consoleFile);
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
#else
    HandleScope scope;
#endif

    // module export to be returned from require(...)
    Local<Object> exports;

    // string representation of module to be required
    String::Utf8Value moduleName(args[0]->ToString());

    // attempt to find the node thread instance by its name
    NativeMap::const_iterator nativeModuleItr =
            threadContext->native_modules->find(*moduleName);
    if(nativeModuleItr != threadContext->native_modules->end())
    {
#if (NODE_MODULE_VERSION > 0x000B)
        exports = NanObjectWrapHandle(nativeModuleItr->second);
#else
        exports = Local<Object>::New(
            NanObjectWrapHandle(nativeModuleItr->second));
#endif
    }

    NanReturnValue(exports);
}