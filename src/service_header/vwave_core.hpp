#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>
#include <queue>
#include <vector>
#include <string.h>
#include <stdarg.h>
#include <initializer_list>
#include <json/json.h>
#include "vm_model.hpp"
#define foreach(src, size) for(int i = 0; i < size; i++)


namespace Exception
{
    extern void HandleException(jvmtiError error);

    extern void HandleException(jvmtiError error, const char *msg);

    extern void HandleExternalException(int error);

    extern void HandleException(jint error);
}

namespace FileTool
{
    extern int Start();

    extern int Stop();

    extern int Output(char *path, char *content, int len);
}

namespace StringTool
{

    struct VString
    {
        char *src;
        int len;
    };

    struct VStringCompareKey
    {
        bool operator()(VString *first, VString *second) const
        {
            return strcmp(first->src, second->src);
        }
    };

    struct CharStrCompareKey
    {
        bool operator()(const char *first, const char *second) const
        {
            return strcmp(first, second);
        }
    };

    void Replace(char *str1, char *str2, char *str3);

    void Concat(char **dest, std::initializer_list<const char*> srcs);

    void Copy(char **dest, const char *source);

    int ConvertJString(jstring input, VString **output);

    int ConvertJString(jstring input, char **dest);

    int DeallocateVString(VString* vstring);

    int DellocateChString(char *str);
}

namespace ThreadTool
{
    typedef void *(*Runnable)(void *);

    extern int StartThread(pthread_t thread, Runnable runnable);

    extern int Test();
}

namespace CollectionTool
{
    extern void ForEachMap();

    template <class T>
    extern void Dequeue(std::queue<T> src, T *data);

    extern void TestForeach();
}

namespace Logger
{
    extern uint8_t Verbose;
    extern uint8_t Debug;
    extern uint8_t Info;
    extern uint8_t Warn;
    extern uint8_t Error;
    extern uint8_t Test;
    extern uint8_t UNKNOWN;

    extern void Init(uint8_t level = Verbose);

    extern void v(char *tag, const char *content);

    extern void d(char *tag, const char *content);

    extern void i(char *tag, const char *content);

    extern void w(char *tag, const char *content);

    extern void e(char *tag, const char *content);

    extern void i(char *tag, jint content);

    extern void Assert(char *tag, const char *content);
}
#endif
