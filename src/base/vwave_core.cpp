#include <pthread.h>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>

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

    
    
    pthread_cond_t output_cond;
    pthread_cond_t input_cond;

    pthread_mutex_t output_mutex;
    pthread_mutex_t input_mutex;

    queue<FileData*> out_queue;
    queue<FileData*> input_queue;

    map<char*, ofstream*> outputs;
    map<char*, ifstream*> inputs;

    int interrupted = 0;

    void *output(void *args) {
        jint error = Global::global_java_vm->AttachCurrentThread(reinterpret_cast<void **>(&Global::global_vm_env), NULL);
        Exception::HandleException(error);
        cout << "currented thread has been attached\n";
        while (!interrupted)
        {
            while (!out_queue.empty())
            {
                if (!pthread_cond_wait(&output_cond, &output_mutex))
                {
                    return (void*)&(EXIT_REASON_UNNORMAL);
                }
            }
            pthread_mutex_lock(&output_mutex);
            FileData *data = out_queue.front();
            (*outputs[data->file])<<data->content;
            delete data;
            out_queue.pop();
            pthread_mutex_unlock(&output_mutex);
        }
        error = Global::global_java_vm->DetachCurrentThread();
        Exception::HandleException(error);
        cout << "currented thread has been dettached\n";
        return (void*)(&EXIT_REASON_NORMAL);
    }

    void *input(void *args) {
        jint error = Global::global_java_vm->AttachCurrentThread(reinterpret_cast<void **>(&Global::global_vm_env), NULL);
        Exception::HandleException(error);
        while (!interrupted)
        {

        }
    }

    int Start()
    {
        interrupted = 0;
        if (pthread_mutex_init(&output_mutex, NULL))
        {
            return 1;
        }
        if (pthread_mutex_init(&input_mutex, NULL))
        {
            return 2;
        }
        if (pthread_cond_init(&output_cond, NULL))
        {
            return 3;
        }
        if (pthread_cond_init(&input_cond, NULL))
        {
            return 4;
        }
        if (pthread_create(&spin_out_thread, NULL, output, NULL))
        {
            return 5;
        }
        if (pthread_create(&spin_out_thread, NULL, input, NULL))
        {
            return 6;
        }
        if (pthread_join(spin_out_thread, NULL))
        {
            return 7;
        }
        if (pthread_join(spin_input_thread, NULL))
        {
            return 8;
        }
        return 0;
    }

    int Stop()
    {
        interrupted = 1;
        if (pthread_cond_signal(&output_cond))
        {
            return 1;
        }
        if (pthread_cond_signal(&input_cond))
        {
            return 2;
        }
        // if (pthread_join(spin_out_thread, NULL))
        // {
        //     return 3;
        // }
        // if (pthread_join(spin_input_thread, NULL))
        // {
        //     return 4;
        // }
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
        pthread_mutex_lock(&output_mutex);
        FileData *data = new FileData();
        data->file = path;
        data->content = content;
        data->len = len;
        out_queue.push(data);
        pthread_mutex_lock(&output_mutex);
        pthread_cond_broadcast(&output_cond);
        return 0;
    }

    int Input(char *path, char *des, int len) 
    {
        return 0;
    }
}