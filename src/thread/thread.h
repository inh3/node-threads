#ifndef _THREAD_H_
#define _THREAD_H_

// libuv
#include <uv.h>

class Thread
{
    public:
        
        static void*        ThreadInit();
        static void         ThreadPostInit(void* threadContext);
        static void         ThreadDestroy(void* threadContext);

    private:

        // libuv
        static void         AsyncCloseCallback(uv_handle_t* handle);
};

#endif /* _THREAD_H_ */