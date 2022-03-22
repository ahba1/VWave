#include <pthread.h>
#include <queue>
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

    queue<FileData*> out_queue;

    map<char*, ofstream*> outputs;

    volatile int interrupted = 0;

    void* _output(void* params) {
        jint error;
        cout << "currented thread has been attached\n";
        while (!interrupted)
        {
            while (!out_queue.empty())
            {
                cout<<"enter loop\n";
                FileData *data = out_queue.front();
                (*outputs[data->file])<<data->content;
                delete data;
                out_queue.pop();
            }
        }
        Exception::HandleException(error);
        cout << "currented thread has been dettached\n";
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
        map<char*, ofstream*>::iterator it = outputs.begin();
        while (it != outputs.end()) {
            it->second->close();
            delete it->second;
        }
        return 0;
    }

    int Output(char *path, char *content, int len)
    {
        cout << path <<endl;
        if (outputs.find(path) == outputs.end())
        {
            ofstream *ofs = new ofstream();
            ofs->open(path);
            outputs[path] = ofs;
        }
        FileData *data = new FileData();
        data->file = path;
        data->content = content;
        data->len = len;
        out_queue.push(data);
        return 0;
    }
}

namespace ThreadTool {
    void* ThreadToolTest(void* params) {
        std::cout<<"ThreadToolTest Method. Current Thread's id is "<<syscall(SYS_gettid)<<std::endl;
    }

    int StartThread(pthread_t thread, Runnable runnable) {
        std::cout<<"StartThread Method. Current Thread's id is "<<syscall(SYS_gettid)<<std::endl;
        int res = pthread_create(&thread, NULL, runnable, NULL);
        if (res)
        {
            std::cout<<"failed to run thread\n";
        }
        return res;        
    }

    int Test() {
        pthread_t thread;
        return StartThread(thread, ThreadToolTest);
    }
}