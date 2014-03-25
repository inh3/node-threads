#define _WORK_ITEM_CC_

#include "work-item.h"

// threadpool
#include "synchronize.h"

// custom
#include "thread.h"
#include "callback-manager.h"
#include "file_info.h"
#include "utilities.h"

// callback manager
static CallbackManager* callbackManager = &(CallbackManager::GetInstance());

Persistent<Function> WorkItem::_Guid;

WorkItem::WorkItem(
    Handle<Function> callbackFunction,
    Handle<Object> workOptions,
    Handle<Object> calleeObject,
    Handle<Object> nodeThreads)
{
    printf("WorkItem::WorkItem\n");
    _WorkResult = NULL;
    _Exception = NULL;

#if (NODE_MODULE_VERSION > 0x000B)
    _CallbackFunction.Reset(Isolate::GetCurrent(), callbackFunction);
    _NodeThreads.Reset(Isolate::GetCurrent(), nodeThreads);
#else
    _CallbackFunction = Persistent<Function>::New(callbackFunction);
    _NodeThreads = Persistent<Object>::New(nodeThreads);
#endif

    ProcessWorkOptions(workOptions);

    String::Utf8Value fileNameStr(calleeObject->Get(
        String::NewSymbol("__filename")));
    String::Utf8Value dirNameStr(calleeObject->Get(
        String::NewSymbol("__dirname")));

    _FileName.assign(*fileNameStr);
    _DirName.assign(*dirNameStr);
}

// 'delete' can only be called from main thread
WorkItem::~WorkItem()
{
    static int x = 0;
    printf("WorkItem::~WorkItem - %u\n", ++x);

    if(_WorkResult != NULL)
    {
        free(_WorkResult);
    }

    if(_Exception != NULL)
    {
        free(_Exception);
    }

    _WorkOptions.Dispose();
    _WorkOptions.Clear();

    _CallbackFunction.Dispose();
    _CallbackFunction.Clear();

    _NodeThreads.Dispose();
    _NodeThreads.Clear();
}

void WorkItem::ProcessWorkOptions(Handle<Object> workOptions)
{
    NanScope();

    if(workOptions.IsEmpty())
    {
        workOptions = Object::New();
    }

    // set context to default node context if not specified
    Handle<Value> contextHandle = workOptions->Get(String::NewSymbol("context"));
    if(contextHandle == Undefined())
    {
        Handle<Object> currentContext = Context::GetCurrent()->Global();
        workOptions->Set(String::NewSymbol("context"), currentContext);
    }

    // set work id to guid if not specified
    Handle<Value> workId = workOptions->Get(String::NewSymbol("id"));
    if(workId == Undefined())
    {
#if (NODE_MODULE_VERSION > 0x000B)
        Local<Function> guidFunction = Local<Function>::New(
            Isolate::GetCurrent(),
            _Guid);
#else
        Local<Function> guidFunction = Local<Function>::New(
            _Guid);
#endif

        Handle<Value> guidHandle = guidFunction->Call(
            Context::GetCurrent()->Global(),
            0,
            NULL);
        workOptions->Set(String::NewSymbol("id"), guidHandle);
    }

#if (NODE_MODULE_VERSION > 0x000B)
    _WorkOptions.Reset(Isolate::GetCurrent(), workOptions);
#else
    _WorkOptions = Persistent<Object>::New(workOptions);
#endif
}

void WorkItem::AsyncCallback(
    Handle<Value> errorHandle,
    Handle<Value> infoHandle,
    Handle<Value> resultHandle)
{
#if (NODE_MODULE_VERSION > 0x000B)
    HandleScope scope(Isolate::GetCurrent());
    Local<Object> nodeThreads = Local<Object>::New(
            Isolate::GetCurrent(),
            _NodeThreads);
#else
    HandleScope scope;
     Local<Object> nodeThreads = Local<Object>::New(
            _NodeThreads);
#endif

    Handle<Object> dataObject = Object::New();
    dataObject->Set(String::NewSymbol("error"), errorHandle);
    dataObject->Set(String::NewSymbol("info"), infoHandle);
    dataObject->Set(String::NewSymbol("result"), resultHandle);

    Handle<Value> eventHandle = Object::New();
    eventHandle.As<Object>()->Set(String::NewSymbol("data"), dataObject);

    Handle<Function> emitFunction = nodeThreads->Get(
        String::NewSymbol("emit"))
    .As<Function>();

     // emit "message" on main thread
    Handle<Value> args[] = { 
        String::New("message"),
        eventHandle
    };
    emitFunction->Call(nodeThreads, 2, args);
}

// static methods -------------------------------------------------------------

void WorkItem::Initialize(const char* moduleDir)
{
    NanScope();

    string guidPath;
    guidPath.assign(moduleDir);
    guidPath.append("/src/js/guid.js");
    FileInfo guidFile(guidPath.c_str());
    Handle<Script> guidScript = Script::New(
        String::New(guidFile.FileContents()),
        String::New("guid"));
    Handle<Value> guidFunction = guidScript->Run();

#if (NODE_MODULE_VERSION > 0x000B)
    _Guid.Reset(Isolate::GetCurrent(), guidFunction.As<Function>());
#else
    _Guid = Persistent<Function>::New(guidFunction.As<Function>());
#endif
}

void* WorkItem::WorkFunction(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    // get reference to the thread context
    thread_context_t* threadContext = (thread_context_t*)threadContextPtr;

    // get refrence to work item
    WorkItem* workItem = (WorkItem*)workItemPtr;

    // get currently running thread id
    /*printf("[ Thread Pool Key ] %s\n",
        threadContext->nodeThreads->GetThreadPoolKey().c_str());*/
    workItem->_ThreadId = SyncGetThreadId();
    workItem->_ThreadPoolKey.assign(
        threadContext->nodeThreads->GetThreadPoolKey().c_str());

    // get reference to thread isolate
    Isolate* isolate = threadContext->thread_isolate;
    {
        // lock the isolate
        Locker isolateLocker(isolate);

        // enter the isolate
        isolate->Enter();

// Node 0.11+ (0.11.3 and below won't compile with these)
#if (NODE_MODULE_VERSION > 0x000B)
        HandleScope handleScope(isolate);
        Context::Scope contextScope(isolate, threadContext->isolate_context);
        Handle<Object> contextObject = isolate->GetCurrentContext()->Global();
#else
        HandleScope handleScope;
        Context::Scope contextScope(threadContext->isolate_context);
        Handle<Object> contextObject = Context::GetCurrent()->Global();
#endif

        // set __filename and __dirname
        contextObject->Set(
            String::NewSymbol("__filename"),
            String::New(workItem->_FileName.c_str()));

        contextObject->Set(
            String::NewSymbol("__dirname"),
            String::New(workItem->_DirName.c_str()));
        
        workItem->InstanceWorkFunction();
    }

    // leave the isolate
    isolate->Exit();

    return (void*)workItem;
}

void WorkItem::WorkCallback(
    TASK_QUEUE_WORK_DATA *taskInfoPtr,
    void *threadContextPtr,
    void *workItemPtr)
{
    WorkItem* workItem = (WorkItem*)workItemPtr;

    static int x = 0;
    printf("WorkItem::WorkCallback - %u\n", ++x);

    // context will be null if this is due to the thread pool
    // being destroyed, so don't execute the actual callback function
    // this will only occur on main thread since thread pool is destroying
    if(threadContextPtr == NULL)
    {
        delete workItem;
    }
    else
    {
        // perform instance callback
        workItem->InstanceWorkCallback();

        // add to callback queue
        callbackManager->AddWorkItem(workItem);

        // send async to main thread
        thread_context_t* threadContext = (thread_context_t*)threadContextPtr;
        uv_async_send(threadContext->uv_async_ptr);
    }
}