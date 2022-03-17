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

    
    
    pthread_cond_t output_cond;
    pthread_cond_t input_cond;

    pthread_mutex_t output_mutex;
    pthread_mutex_t input_mutex;

    queue<FileData*> out_queue;
    queue<FileData*> input_queue;

    map<char*, ofstream*> outputs;
    map<char*, ifstream*> inputs;

    int interrupted = 0;

    void JNICALL output(jvmtiEnv* jvmti_env, JNIEnv* jni_env, void* arg) {
        jint error;
        cout << "currented thread has been attached\n";
        while (!interrupted)
        {
            while (!out_queue.empty())
            {
                FileData *data = out_queue.front();
                (*outputs[data->file])<<data->content;
                delete data;
                out_queue.pop();
            }
        }
        error = Global::global_java_vm->DetachCurrentThread();
        Exception::HandleException(error);
        cout << "currented thread has been dettached\n";
    }

    int Start()
    {
        interrupted = 0;
        Global::global_vm_env->RunAgentThread(*Global::global_agent_thread, output, NULL, JVMTI_THREAD_NORM_PRIORITY);
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