#define _ERROR_HANDLING_CC_

#include "error-handling.h"

// https://code.google.com/p/v8/source/browse/trunk/samples/shell.cc
Handle<Object> ErrorHandling::HandleException(TryCatch* tryCatch)
{
    // exception object to return
    Handle<Object> exceptionObject = NanNew<Object>();

    // get the exception message
    Handle<Message> exceptionMessage = tryCatch->Message();

    // the exception message was not valid
    if (exceptionMessage.IsEmpty())
    {
        exceptionObject->Set(NanNew<String>("message"), tryCatch->Exception());
    }
    else
    {
        exceptionObject->Set(NanNew<String>("message"), tryCatch->Message()->Get());
        exceptionObject->Set(NanNew<String>("resourceName"), exceptionMessage->GetScriptResourceName());
        exceptionObject->Set(NanNew<String>("lineNum"), NanNew<Number>(exceptionMessage->GetLineNumber()));
        exceptionObject->Set(NanNew<String>("sourceLine"), exceptionMessage->GetSourceLine());
        exceptionObject->Set(NanNew<String>("scriptData"), exceptionMessage->GetScriptData());
        if(!tryCatch->StackTrace().IsEmpty())
        {
            exceptionObject->Set(NanNew<String>("stackTrace"), tryCatch->StackTrace());
        }
        else
        {
            exceptionObject->Set(NanNew<String>("stackTrace"), NanNull());
        }
    }

    exceptionObject->SetHiddenValue(NanNew<String>("exception"), NanTrue());
    return exceptionObject;
}
