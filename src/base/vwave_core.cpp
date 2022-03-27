#include <pthread.h>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>

#include "vwave_core.hpp"
#include "../global.hpp"

namespace Exception
{
    void HandleException(jvmtiError error)
    {
        switch (error)
        {
        case JVMTI_ERROR_NONE:
            break;
        default:
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

namespace FileTool
{
    struct FileData
    {
        char *file;
        char *content;
        int len;
    };

    int EXIT_REASON_NORMAL = 0;
    int EXIT_REASON_UNNORMAL = 1;

    pthread_t spin_out_thread;
    pthread_t spin_input_thread;

    queue<FileData *> out_queue;
    map<StringTool::VString *, std::ofstream *, StringTool::VStringCompareKey> outputs;

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
                FileData *data = out_queue.front();
                out_queue.pop();
                cout<<data->file;
                cout<< data->content;
                StringTool::VString *vpath;
                error = Global::global_vm_env->Allocate(sizeof(StringTool::VString), reinterpret_cast<unsigned char**>(&vpath));
                Exception::HandleException(error);
                vpath->src=data->file;
                vpath->len=strlen(data->file);
                if (outputs.count(vpath) == 0)
                {
                    std::ofstream *ofs;
                    error = Global::global_vm_env->Allocate(sizeof(std::ofstream), reinterpret_cast<unsigned char**>(&ofs));
                    Exception::HandleException(error);
                    ofs->open(data->file);
                    outputs.insert(make_pair(vpath, ofs));
                    // cout <<"\n";
                    // cout << "open file" << endl;
                    // cout <<"\n";
                    (*outputs[vpath]) << data->content;
                    count++;
                } else {
                    (*outputs[vpath]) << data->content;
                    count++;
                    if (count == 20)
                    {
                        count=0;
                        outputs[vpath]->flush();
                    }
                    error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(vpath));
                }
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data->file));
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data->content));
                error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(data));
                Exception::HandleException(error);
            }
        }
        map<StringTool::VString *, std::ofstream *>::iterator it = outputs.begin();
        while (it != outputs.end())
        {
            it->second->close();
            error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(it->second));
            error = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char*>(it->first));
            Exception::HandleException(error);
        }
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
        FileData *data;
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(FileData), reinterpret_cast<unsigned char **>(&data));
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
            std::cout << "failed to run thread\n";
        }
        return res;
    }

    int Test()
    {
        pthread_t thread;
        return StartThread(thread, ThreadToolTest);
    }
}