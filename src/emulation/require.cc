#define _REQUIRE_CC_

#include "require.h"

// C
#include <stdio.h>

// custom
#include "thread.h"

NAN_METHOD(Require::RequireMethod)
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

    printf("Require Function!\n");

    NanReturnUndefined();
}