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

#include "nan-extra.h"

void Require::InitializePerIsolate()
{
    string modulePath;

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/util.js");
    FileInfo utilFile(modulePath.c_str());
    LoadNativeModule(NanNew<String>("util"), &utilFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/path.js");
    FileInfo pathFile(modulePath.c_str());
    LoadNativeModule(NanNew<String>("path"), &pathFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/assert.js");
    FileInfo assertFile(modulePath.c_str());
    LoadNativeModule(NanNew<String>("assert"), &assertFile);

    modulePath.assign(NTEnvironment::ModuleDir);
    modulePath.append("/src/js/console.js");
    FileInfo consoleFile(modulePath.c_str());
    LoadNativeModule(NanNew<String>("console"), &consoleFile);
}

void Require::LoadNativeModule(Handle<String> moduleName, FileInfo* nativeFileInfo)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);

    NanScope();

    // get handles to module context and the isolate context
    Handle<Context> moduleContext = NanNewContextHandle();
    Handle<Context> isolateContext = NanGetCurrentContext();

    // set the security token to access calling context properties within new context
    moduleContext->SetSecurityToken(isolateContext->GetSecurityToken());

    // enter module context scope
    moduleContext->Enter();

    // get reference to current context's object
    Handle<Object> contextObject = moduleContext->Global();

    // clone the calling context properties into this context
    ThreadIsolate::CloneGlobalContext(isolateContext->Global(), contextObject);

    // create the module context
    ThreadIsolate::CreateModuleContext(contextObject, nativeFileInfo);

    // run the script to get the module object
    ScriptOrigin scriptOrigin(NanNew<String>(moduleName));

    #if NODE_VERSION_AT_LEAST(0, 11, 13)
    Handle<UnboundScript> moduleScript = NanNew<NanUnboundScript>(
        NanNew<String>(nativeFileInfo->FileContents()),
        scriptOrigin);
    #else
    Handle<Script> moduleScript = NanNew<Script>(
        NanNew<String>(nativeFileInfo->FileContents()),
        scriptOrigin);
    #endif
    NanRunScript(moduleScript);

    Handle<Object> moduleObject = contextObject->Get(NanNew<String>("module"))->ToObject();

    // create object template in order to use object wrap
    Handle<ObjectTemplate> objectTemplate = ObjectTemplate::New();
    objectTemplate->SetInternalFieldCount(1);
    Local<Object> moduleInstance = objectTemplate->NewInstance();

    // copy the module object to module instance
    Utilities::CopyObject(
        moduleInstance,
        moduleObject->Get(NanNew<String>("exports"))->ToObject());

    // wrap the object so it can be persisted
    PersistentWrap* moduleWrap = new PersistentWrap();
    moduleWrap->Wrap(moduleInstance);
    moduleWrap->Ref();

    // add to the thread context for later lookup
    String::Utf8Value moduleNameStr(moduleName);
    threadContext->native_modules->insert(make_pair(*moduleNameStr, moduleWrap));

    // exit module context scope
    moduleContext->Exit();
}

// node -----------------------------------------------------------------------

NAN_METHOD(Require::RequireMethod)
{
    // get reference to current isolate
    Isolate* isolate = Isolate::GetCurrent();

    // get reference to thread context for this isolate
    thread_context_t *threadContext = (thread_context_t*)IsolateGetData(isolate);

    NanScope();

    // module export to be returned from require(...)
    Local<Object> exports;

    // string representation of module to be required
    String::Utf8Value moduleName(args[0]->ToString());

    // attempt to find the node thread instance by its name
    NativeMap::const_iterator nativeModuleItr =
            threadContext->native_modules->find(*moduleName);
    if(nativeModuleItr != threadContext->native_modules->end())
    {
        exports = NanNew(NanObjectWrapHandle(nativeModuleItr->second));
    }

    NanReturnValue(exports);
}
