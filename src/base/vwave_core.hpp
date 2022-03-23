#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>

namespace Exception {
    extern void HandleException(jvmtiError error);

    extern void HandleException(int error);

    extern void HandleException(jint error);
}

namespace FileTool {
    extern int Start();

    extern int Stop();

    extern int Output(char *path, char *content, int len);
}

namespace StringTool {
    class VString;
}

namespace ThreadTool {
    typedef void* (*Runnable)(void*);

    extern int StartThread(pthread_t thread, Runnable runnable);

    extern int Test();
}
#endif
