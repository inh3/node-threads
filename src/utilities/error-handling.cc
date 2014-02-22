#define _ERROR_HANDLING_CC_

#include "error-handling.h"

// https://code.google.com/p/v8/source/browse/trunk/samples/shell.cc
Handle<Object> ErrorHandling::HandleException(TryCatch* tryCatch)
{
    // exception object to return
    Handle<Object> exceptionObject = Object::New();

    // get the exception message
    Handle<Message> exceptionMessage = tryCatch->Message();

    // the exception message was not valid
    if (exceptionMessage.IsEmpty())
    {
        exceptionObject->Set(String::NewSymbol("message"), tryCatch->Exception());
    }
    else
    {
        exceptionObject->Set(String::NewSymbol("message"), tryCatch->Message()->Get());
        exceptionObject->Set(String::NewSymbol("resourceName"), exceptionMessage->GetScriptResourceName());
        exceptionObject->Set(String::NewSymbol("lineNum"), Number::New(exceptionMessage->GetLineNumber()));
        exceptionObject->Set(String::NewSymbol("sourceLine"), exceptionMessage->GetSourceLine());
        exceptionObject->Set(String::NewSymbol("scriptData"), exceptionMessage->GetScriptData());
        if(!tryCatch->StackTrace().IsEmpty())
        {
            exceptionObject->Set(String::NewSymbol("stackTrace"), tryCatch->StackTrace());
        }
        else
        {
            exceptionObject->Set(String::NewSymbol("stackTrace"), Null());
        }
    }

    exceptionObject->SetHiddenValue(String::New("exception"), Boolean::New(true));
    return exceptionObject;
}