#ifndef _THREAD_H_
#define _THREAD_H_

class Thread
{
    public:
        
        static void*                ThreadInit();
        static void                 ThreadPostInit(void* threadContext);
        static void                 ThreadDestroy(void* threadContext);
};

#endif /* _THREAD_H_ */