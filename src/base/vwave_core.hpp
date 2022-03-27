#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>
#include <queue>
#include <string.h>

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

    struct VString
    {
        char *src;
        int len;
    };

    struct VStringCompareKey
    {
        bool operator()(VString* first, VString* second) const
        {
            return strcmp(first->src, second->src);
        }
    };
}

namespace ThreadTool {
    typedef void* (*Runnable)(void*);

    extern int StartThread(pthread_t thread, Runnable runnable);

    extern int Test();
}

namespace CollectionTool {
    extern void ForEachMap();

    template<class T>
    extern void Dequeue(std::queue<T> src, T *data);
}
#endif
