#include <pthread.h>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <chrono>
#include <execinfo.h>

#include "vwave_core.hpp"
#include "../global.hpp"

#define RED "\033[31m"
#define YELLOW "\033[33m"


namespace Exception
{
    void PrintStackTrace()
    {
        void *buffer[Global::_stack_trace_depth] = {0};
        int num = backtrace(buffer, Global::_stack_trace_depth);
        char **symbols = backtrace_symbols(buffer, num);
        if(symbols)
        {
            for(int i=0;i<num;i++)
            {
                std::cout<< symbols[i] <<std::endl;
            }
            delete symbols;
        }
    }

    void HandleException(jvmtiError error)
    {
        switch (error)
        {
        case JVMTI_ERROR_NONE:
            break;
        default:
            PrintStackTrace();
            std::cout << "internal exception happened\n";
            std::cout << error << "\n";
            throw error;
        }
    }

    void HandleExternalException(int error)
    {
        switch (error)
        {
        case 0:
            break;
        default:
            std::cout << "external exception happened\n";
            std::cout << error << "\n";
            throw error;
        }
    }

    void HandleInternalException(jint error)
    {
        switch (error)
        {
        case JNI_OK:
            break;
        default:
            std::cout << "internal exception happened\n";
            std::cout << error << "\n";
            throw error;
        }
    }
}

namespace StringTool
{
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

    void Concat(char **dest, const char* src, ...)
    {
        // va_list va_src;
        // va_start(va_src, src);

        // while (va_arg(va_src, int) != -1)
        // {

        // }
        // if (src.size() < 1)
        // {
        //     return;
        // }
        // jvmtiError e;
        // int len = 0;
        // for (auto p = src.begin(); p != src.end(); p++)
        // {
        //     len += strlen(*p);
        // }
        // e = Global::global_vm_env->Allocate(len + 1, reinterpret_cast<Global::memory_alloc_ptr>(dest));
        // Exception::HandleException(e);
        // auto begin = src.begin();
        // strcpy(*dest, *begin);
        // begin++;
        // for (; begin != src.end(); begin++)
        // {
        //     strcat(*dest, *begin);
        // }
    }
}

namespace FileTool
{
    struct CharStrFileData
    {
        char *file;
        char *content;
        int len;
    };

    int EXIT_REASON_NORMAL = 0;
    int EXIT_REASON_UNNORMAL = 1;

    pthread_t spin_out_thread;
    pthread_t spin_input_thread;

    static queue<CharStrFileData *> out_queue;
    static map<char*, std::ofstream, StringTool::CharStrCompareKey> outputs;

    volatile int interrupted = 0;

    void closeFile()
    {

    }

    void *_output(void *params)
    {
        jvmtiError error;
        int count = 0;
        while (!interrupted)
        {
            while (!out_queue.empty())
            {
                CharStrFileData *data = out_queue.front();
                //Logger::d("FileTool output", data->content);
                std::ofstream ofs(data->file, ios::app);
                ofs << data->content;
                ofs.close();            
                out_queue.pop();
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data->file));
                Exception::HandleException(error);
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data->content));
                Exception::HandleException(error);
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data));
                Exception::HandleException(error);
                // if (outputs.count(data->file) == 0)
                // {
                //     Logger::d("FileTool", "openfile");
                //     std::ofstream ofs;
                //     outputs.insert(make_pair(data->file, ofs));
                //     outputs[data->file] << data->content;
                //     count++;
                // } else {
                //     Logger::d("FileTool", "file has opened");
                //     Logger::d("FileTool", data->file);
                //     outputs[data->file] << data->content;
                //     count++;
                //     if (count == 20)
                //     {
                //         count=0;
                //         outputs[data->file].flush();
                //     }
                // }
            }
        }
        // map<char*, std::ofstream>::iterator it = outputs.begin();
        // while (it != outputs.end())
        // {
        //     it->second.close();
        //     error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(it->first));
        //     Exception::HandleException(error);
        // }
    }

    int Start()
    {
        ThreadTool::StartThread(spin_out_thread, _output);
        interrupted = 0;
        return 0;
    }

    int Stop()
    {
        interrupted = 1;
        pthread_join(spin_out_thread, NULL);
        cout << "exit main thread\n";
        return 0;
    }

    int Output(char *path, char *content, int len)
    {
        CharStrFileData *data;
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(CharStrFileData), reinterpret_cast<unsigned char **>(&data));
        Exception::HandleException(error);
        data->file = path;
        data->content = content;
        data->len = len;
        out_queue.push(data);
        return 0;
    }
}

namespace ThreadTool
{
    void *ThreadToolTest(void *params)
    {
        std::cout << "ThreadToolTest Method. Current Thread's id is " << syscall(SYS_gettid) << std::endl;
    }

    int StartThread(pthread_t thread, Runnable runnable)
    {
        // std::cout << "StartThread Method. Current Thread's id is " << syscall(SYS_gettid) << std::endl;
        int res = pthread_create(&thread, NULL, runnable, NULL);
        if (res)
        {
            Logger::e("ThreadTool", "Start Thread failed");
        }
        return res;
    }

    int Test()
    {
        pthread_t thread;
        return StartThread(thread, ThreadToolTest);
    }
}

namespace Logger 
{
    uint8_t Verbose = 1;
    uint8_t Debug = 1 << 1;
    uint8_t Info = 1 << 2;
    uint8_t Warn = 1 << 3;
    uint8_t Error = 1 << 4;
    uint8_t UNKNOWN = 0;
    uint8_t CurrentLevel = UNKNOWN;

    void Init(uint8_t level)
    {
        std::cout << level << std::endl;
        CurrentLevel = level;
    }

    void _InterOut(char *tag, char *content, char *color = NULL)
    {
        auto now = std::chrono::system_clock::now();
        uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count() - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
        time_t tt = std::chrono::system_clock::to_time_t(now);
        auto time_tm = localtime(&tt);
        char strTime[25] = {0};
        sprintf(strTime, 
            "%d-%02d-%02d %02d:%02d:%02d %03d", 
            time_tm->tm_year + 1900,
            time_tm->tm_mon + 1,
            time_tm->tm_mday,
            time_tm->tm_hour,
            time_tm->tm_min,
            time_tm->tm_sec,
            (int)dis_millseconds
        );
        std::cout << strTime << " : " << tag << "  " << content << std::endl;
    }

    void v(char *tag, char *content)
    {
        if (CurrentLevel & Verbose)
        {
            _InterOut(tag, content);
        }
    }

    void d(char *tag, char *content)
    {
        if (CurrentLevel & Debug)
        {
            _InterOut(tag, content);
        }
    }

    void i(char *tag, char *content)
    {
        if (CurrentLevel & Info)
        {
            _InterOut(tag, content);
        }
    }

    void w(char *tag, char *content)
    {
        if (CurrentLevel & Warn)
        {
            _InterOut(tag, content, YELLOW);
        }
    }

    void e(char *tag, char *content)
    {
        if (CurrentLevel & Error)
        {
            _InterOut(tag, content, RED);
        }
    }
}