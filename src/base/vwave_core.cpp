#include <pthread.h>
#include <queue>
#include <map>
#include <iostream>
#include <string.h>
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

namespace StringTool
{
    class VString
    {
    private:
        char *_src;
        int len;
    public:
        VString(char *src)
        {
            jvmtiError e = Global::global_vm_env->Allocate(strlen(src), reinterpret_cast<unsigned char **>(&_src));
            strcpy(_src, src);
            len = strlen(_src);
            Exception::HandleException(e);
        }
        VString()
        {
            _src = NULL;
            len = 0;
        }
        ~VString()
        {
            jvmtiError e = Global::global_vm_env->Deallocate(reinterpret_cast<unsigned char *>(_src));
            Exception::HandleException(e);
        }
        VString operator+(const VString &vstr) const
        {
            VString res;
            jvmtiError e = Global::global_vm_env->Allocate(vstr.len + len, reinterpret_cast<unsigned char**>(&res._src));
            Exception::HandleException(e);
            strcpy(res._src, _src);
            strcat(res._src, vstr._src);
            res.len = strlen(res._src);
            return res;
        }
        VString operator+(const char* &str) const
        {
            VString res;
            jvmtiError e = Global::global_vm_env->Allocate(strlen(str) + len, reinterpret_cast<unsigned char**>(&res._src));
            Exception::HandleException(e);
            strcpy(res._src, _src);
            strcat(res._src, str);
            return res;
        }
        bool operator<(const VString &vstr) const
        {
            cout << _src << " " << vstr._src << endl;
            cout << (strcmp(vstr._src, _src) < 0) <<endl;
            cout << (strcmp(vstr._src, _src) > 0) <<endl;
            return strcmp(vstr._src, _src) < 0;
        }
        bool operator<(const char* &str) const
        {
            return strcmp(str, _src) < 0;
        }
        friend ostream & operator<<(ostream &out, VString &vstr)
        {
            out << vstr._src;
            return out;
        }
    };
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


    volatile int interrupted = 0;

    void *_output(void *params)
    {
        jint error;
        cout << "currented thread has been attached\n";
        while (!interrupted)
        {
            while (!out_queue.empty())
            {
                // cout << "enter loop\n";
                // FileData *data = out_queue.front();
                // //(*outputs[data->file]) << data->content;
                // delete data;
                // out_queue.pop();
                // cout << "exit loop\n";
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
        pthread_join(spin_out_thread, NULL);
        cout << "exit main thread\n";
        // map<StringTool::VString, ofstream *>::iterator it = outputs.begin();
        // while (it != outputs.end())
        // {
        //     it->second->close();
        //     delete it->second;
        // }
        return 0;
    }

    int Output(char *path, char *content, int len)
    {
        StringTool::VString vpath(path);
        // if (outputs.count(vpath) == 0)
        // {
        //     ofstream *ofs = new ofstream();
        //     ofs->open(path);
        //     outputs[vpath] = ofs;
        //     cout << "open file" << endl;
        // }
        // FileData *data = new FileData();
        // data->file = path;
        // data->content = content;
        // data->len = len;
        // out_queue.push(data);
        // cout << "output end\n";
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
        //std::cout << "StartThread Method. Current Thread's id is " << syscall(SYS_gettid) << std::endl;
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