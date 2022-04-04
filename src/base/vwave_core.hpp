#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>
#include <queue>
#include <string.h>

namespace Exception
{
    extern void HandleException(jvmtiError error);

    extern void HandleException(int error);

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
        bool operator()(char *first, char *second) const
        {
            return strcmp(first, second);
        }
    };

    void Replace(char *str1, char *str2, char *str3)
    {
        int i, j, k, done, count = 0, gap = 0;
        char temp[100];
        for (i = 0; i < strlen(str1); i += gap)
        {
            if (str1[i] == str2[0])
            {
                done = 0;
                for (j = i, k = 0; k < strlen(str2); j++, k++)
                {
                    if (str1[j] != str2[k])
                    {
                        done = 1;
                        gap = k;
                        break;
                    }
                }
                if (done == 0)
                { 
                    for (j = i + strlen(str2), k = 0; j < strlen(str1); j++, k++)
                    { 
                        temp[k] = str1[j];
                    }
                    temp[k] = '\0'; 
                    for (j = i, k = 0; k < strlen(str3); j++, k++)
                    { 
                        str1[j] = str3[k];
                        count++;
                    }
                    for (k = 0; k < strlen(temp); j++, k++)
                    { 
                        str1[j] = temp[k];
                    }
                    str1[j] = '\0'; 
                    gap = strlen(str2);
                }
            }
            else
            {
                gap = 1;
            }
        }
        if (count == 0)
        {
            printf("Can't find the replaced string!\n");
        }
        return;
    }
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
}

namespace Logger
{
    extern uint8_t Verbose;
    extern uint8_t Debug;
    extern uint8_t Info;
    extern uint8_t Warn;
    extern uint8_t Error;
    extern uint8_t UNKNOWN;

    extern void Init(uint8_t level = Verbose);

    extern void v(char *tag, char *content);

    extern void d(char *tag, char *content);

    extern void i(char *tag, char *content);

    extern void w(char *tag, char *content);

    extern void e(char *tag, char *content);
}
#endif
