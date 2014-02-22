#define _UTILITIES_CC_

#include "utilities.h"
#include "nan.h"

// C
#include <stdio.h>

// custom
#include "error-handling.h"

void Utilities::PrintObjectProperties(Handle<Object> objectHandle)
{
    Local<Array> propertyKeys = (*objectHandle)->GetPropertyNames();
    for (uint32_t keyIndex = 0; keyIndex < propertyKeys->Length(); keyIndex++)
    {
        Handle<v8::String> keyString = propertyKeys->Get(keyIndex)->ToString();
        String::AsciiValue propertyName(keyString);
        fprintf(stdout, "[ Property %u ] %s\n", keyIndex, *propertyName);
    }
}

Handle<Value> Utilities::CompileScriptSource(Handle<String> scriptSource, const char* scriptResourceName)
{
    TryCatch tryCatch;

    Handle<Value> scriptResult;
    Handle<Script> compiledScript;
    if(scriptResourceName != NULL)
    {
        compiledScript = Script::New(scriptSource, String::New(scriptResourceName));
    }
    else
    {
        compiledScript = Script::New(scriptSource);
    }

    // check for exception on compile
    if(compiledScript.IsEmpty() || tryCatch.HasCaught())
    {
        return ErrorHandling::HandleException(&tryCatch);
    }
    else
    {
        scriptResult = compiledScript->Run();

        // check that running script didn't throw errors
        if(scriptResult.IsEmpty() || tryCatch.HasCaught())
        {
            return ErrorHandling::HandleException(&tryCatch);
        }
    }

    return scriptResult;
}