#define _UTILITIES_CC_

#include "utilities.h"

// C
#include <stdio.h>

// custom
#include "error-handling.h"

// https://code.google.com/p/v8/source/browse/trunk/samples/shell.cc#91
const char* Utilities::ToCString(const String::Utf8Value& value)
{
    return *value ? *value : "<string conversion failed>";
}

void Utilities::PrintObjectProperties(Handle<Object> objectHandle)
{
    Local<Array> propertyKeys = objectHandle->GetPropertyNames();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Handle<v8::String> keyString = propertyKeys->Get(keyIndex)->ToString();
        
        String::Utf8Value propertyName(keyString);
        String::Utf8Value propertyValue(objectHandle->Get(keyString)->ToString());
        fprintf(stdout, "[ Property %u ] %s - %s\n",
            keyIndex,
            *propertyName,
            *propertyValue);
    }
}

void Utilities::CopyObject(Handle<Object> toObject, Handle<Object> fromObject)
{
    Local<Array> propertyKeys = fromObject->GetPropertyNames();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Handle<Value> propertyKey = propertyKeys->Get(keyIndex);
        toObject->Set(propertyKey, fromObject->Get(propertyKey));
    }
}

Handle<Value> Utilities::CompileScriptSource(
    Handle<String> scriptSource,
    const char* scriptResourceName)
{
    TryCatch tryCatch;

    Handle<Value> scriptResult;
    Handle<UnboundScript> unboundScript;

    // create compiled script
    if(scriptResourceName != NULL)
    {
        ScriptOrigin scriptOrigin(NanNew<String>(scriptResourceName));
        unboundScript = NanNew<NanUnboundScript>(
            NanNew<String>(scriptSource),
            scriptOrigin);
    }
    else
    {
        unboundScript = NanNew<NanUnboundScript>(NanNew<String>(scriptSource));
    }

    // check for exception on compile
    if(unboundScript.IsEmpty() || tryCatch.HasCaught())
    {
        return ErrorHandling::HandleException(&tryCatch);
    }
    else
    {
        scriptResult = NanRunScript(unboundScript);

        // check that running script didn't throw errors
        if(scriptResult.IsEmpty() || tryCatch.HasCaught())
        {
            return ErrorHandling::HandleException(&tryCatch);
        }
    }

    return scriptResult;
}
