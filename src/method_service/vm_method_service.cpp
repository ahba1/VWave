#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <stack>
#include <set>
#include <regex>
#include <chrono>
#include <sstream>
#include <vector>

#include <shared_func.h>

#include "../service_header/vm_method_service.hpp"
#include "../service_header/vm_frame_service.hpp"
#include "vm_param_watch.hpp"
#include "vm_method_invoke.hpp"

namespace VMModel
{
    /**
     * @brief
     * format: [access] [static or not] [final or not] [generic] synchronized/native [return-type] [name]([param-type...])
     */
    void PrintJMethod(Method *method)
    {
        cout << method->access_flag << " " << (method->generic ? (method->generic) : "") << " " << (method->is_native ? "native" : "") << " " << method->signature << " " << method->name << "\n";
    }
}

namespace VMMethodService
{
    map<string, vector<VMMethodHandler>> entry_filters;
    map<string, vector<VMMethodHandler>> exit_filters;
    stack<VMModel::MethodFrame *> method_stack;
    set<char *, StringTool::CharStrCompareKey> white_list;

    char *recording_folder;
    int _recordMethodStarted = 0;
    const char *_cost_file = "./cost.txt";

    uint32_t block_count = 0;

    VMModel::MethodTask *task = NULL;

    void _SetMethodEntryTime(char *method)
    {
        VMModel::MethodFrame *mf;
        VMModel::CreateMethodFrame(&mf, method);
        method_stack.push(mf);
    }

    void _ResolveMethodEntryTime(char *method)
    {
        using namespace std::chrono;
        time_point<high_resolution_clock> _time = high_resolution_clock::now();
        while (!method_stack.empty())
        {
            VMModel::MethodFrame *mf = method_stack.top();
            method_stack.pop();
            if (!strcmp(mf->name, method))
            {
                jvmtiError e;
                duration<double, std::milli> tm = _time - *mf->tm;
                double ms = tm.count();
                ostringstream oss;
                oss << ms;
                const char *ms_str = oss.str().c_str();
                char *midfix = ": ";
                char *suffix = "ms\n";
                int len = strlen(method) + strlen(midfix) + strlen(ms_str) + strlen(suffix) + 1;
                char *content;
                StringTool::Concat(&content, {method, ": ", ms_str, suffix});
                char *path;
                e = Global::global_vm_env->Allocate(strlen(_cost_file), reinterpret_cast<Global::memory_alloc_ptr>(&path));
                Exception::HandleException(e);
                strcpy(path, _cost_file);
                FileTool::Output(path, content, len);
                //Logger::i("MethodCost", content);
            }
            VMModel::DellocateMethodFrame(mf);
        }
    }

    void _RecordVMMethodEntryHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);

        char *file;
        StringTool::Concat(&file, {recording_folder, vm_thread.thread_name, ".txt"});

        VMModel::VMClazz *vm_clazz;
        error = Global::global_vm_env->Allocate(sizeof(VMModel::VMClazz), reinterpret_cast<unsigned char **>(&vm_clazz));
        VMModel::MapJClazz(method->meta->_clazz, &vm_clazz);

        char *full_name;
        VMModel::GetMethodFullName(&full_name, method, vm_clazz);
        _SetMethodEntryTime(full_name);
        char *content;
        StringTool::Concat(&content, {"enter ", full_name, "\n"});
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(full_name));
        Exception::HandleException(error);
        FileTool::Output(file, content, strlen(content));

        VMModel::DellcateClazz(vm_clazz);
        VMModel::DellocateThread(vm_env, &vm_thread);
    }

    void _RecordVMMethodExitHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);

        char *file;
        StringTool::Concat(&file, {recording_folder, vm_thread.thread_name, ".txt"});

        VMModel::VMClazz *vm_clazz;
        error = Global::global_vm_env->Allocate(sizeof(VMModel::VMClazz), reinterpret_cast<unsigned char **>(&vm_clazz));
        VMModel::MapJClazz(method->meta->_clazz, &vm_clazz);

        char *full_name;
        VMModel::GetMethodFullName(&full_name, method, vm_clazz);
        _ResolveMethodEntryTime(full_name);
        char *content;
        StringTool::Concat(&content, {"exit ", full_name, "\n"});
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(full_name));
        Exception::HandleException(error);
        FileTool::Output(file, content, strlen(content));

        VMModel::DellcateClazz(vm_clazz);
        VMModel::DellocateThread(vm_env, &vm_thread);
    }

    void _AllocateCapabilities()
    {
        jvmtiCapabilities caps;
        Logger::i("VMMethodService", "AllocateCapabilities");
        memset(&caps, 0, sizeof(caps));
        caps.can_generate_method_entry_events = 1;
        caps.can_generate_method_exit_events = 1;
        caps.can_get_source_file_name = 1;
        caps.can_access_local_variables = 1;
        jvmtiError e = Global::global_vm_env->AddCapabilities(&caps);
        Exception::HandleException(e);
    }

    void _ParseOptions(char **options, int size)
    {
        int kv_size = 0;
        char **cmd_kv = split(options[0], _spilt_kv_token, _max_kv_size, &kv_size);
        if (!strcmp(cmd_kv[0], "task"))
        {
            task = VMModel::ConvertToMethodTask(cmd_kv[1]);
            if (task == NULL)
            {
                Logger::e("MethodService", "Parse failed");
                return;
            }
            VMModel::PrintMethodTask(task);
            for (int i=0;i<task->filter_len;i++)
            {
                Logger::d("AddFilter", task->method_filter[i]);
                AddEntryFilter(task->method_filter[i], _RecordVMMethodEntryHandler);
                AddExitFilter(task->method_filter[i], _RecordVMMethodExitHandler);
            }
            for (int i=0;i<task->param_read_tasks_len;i++)
            {
                for (int j=0;j<task->param_read_tasks[i].names_len;j++)
                {
                    ParamWatch::SetWatchField(task->param_read_tasks[i].filter,
                     task->param_read_tasks[i].names[j],
                     task->param_read_tasks[i].types[j]);
                }
                //AddEntryFilter(task->param_read_tasks[i].filter, ParamWatch::OnWatchField);
                AddExitFilter(task->param_read_tasks[i].filter, ParamWatch::OnWatchField);
            }
            for (int i=0;i<task->method_invoke_tasks_len;i++)
            {
                MethodInvoke::SetInvokeMethod(task->method_invoke_tasks[i].filter, &task->method_invoke_tasks[i]);
                AddExitFilter(task->method_invoke_tasks->filter, MethodInvoke::OnMethodInvoke);
            }
            RecordMethod("./");
        }
        else 
        {
            Logger::e("MethodService", "unsupprot options key");
        }
    }

    void JNICALL _HandleMethodEntry(
        jvmtiEnv *jvmti_env,
        JNIEnv *jni_env,
        jthread thread,
        jmethodID methodID)
    {
        jvmtiError error;
        VMModel::Method *method;
        VMModel::MapJMethod(methodID, &method);
        map<string, vector<VMMethodHandler>>::iterator it;
        it = entry_filters.begin();
        while (it != entry_filters.end())
        {
            if (white_list.find(method->name) != white_list.end())
            {
                block_count++;
                break;
            }
            if (block_count != 0)
            {
                break;
            }
            if (regex_search(method->name, regex(it->first)))
            {
                for (auto &handler : it->second)
                {
                    handler(jvmti_env, jni_env, thread, method);
                }
            }
            it++;
        }
        VMModel::DellocateMethod(method);
    }

    void JNICALL _HandleMethodExit(
        jvmtiEnv *jvmti_env,
        JNIEnv *jni_env,
        jthread thread,
        jmethodID methodID,
        jboolean was_popped_by_exception,
        jvalue return_value)
    {
        jvmtiError error;
        VMModel::Method *method;
        VMModel::MapJMethod(methodID, &method);
        map<string, vector<VMMethodHandler>>::iterator it;
        it = exit_filters.begin();
        while (it != exit_filters.end())
        {
            if (white_list.find(method->name) != white_list.end())
            {
                block_count--;
                break;
            }
            if (block_count != 0)
            {
                break;
            }
            if (regex_search(method->name, regex(it->first)))
            {
                for (auto &handler : it->second)
                {
                    handler(jvmti_env, jni_env, thread, method);
                }
            }
            it++;
        }
        VMModel::DellocateMethod(method);
    }

    void _InitWhiteList()
    {
        jvmtiError error;
        ifstream ifs;
        string line;
        ifs.open("../config/method_white_list.txt", ios::in);
        if (ifs)
        {
            while (getline(ifs, line))
            {
                const char *c_str = line.c_str();
                char *data;
                error = Global::global_vm_env->Allocate(strlen(c_str) + 1, reinterpret_cast<Global::memory_alloc_ptr>(&data));
                Exception::HandleException(error);
                strcpy(data, c_str);
                white_list.insert(data);
                Logger::i("WhiteList", data);
            }
            ifs.close();
        }
    }

    void _ReleaseWhiteList()
    {
        jvmtiError error;
        for (set<char *>::iterator it = white_list.begin(); it != white_list.end(); it++)
        {
            error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(*it));
            Exception::HandleException(error);
        }
        white_list.clear();
    }

    void Init(char **options, int option_size)
    {
        _AllocateCapabilities();
        _InitWhiteList();
        _ParseOptions(options, option_size);
    }

    void DefaultVMMethodHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        VMModel::PrintJMethod(method);
    }

    void AddEntryFilter(char *filter, VMMethodHandler handler)
    {
        if (entry_filters.count(filter) == 0)
        {
            entry_filters[filter] = vector<VMMethodHandler>();
        }
        entry_filters[filter].push_back(handler);
    }

    void AddExitFilter(char *filter, VMMethodHandler handler)
    {
        if (exit_filters.count(filter) == 0)
        {
            exit_filters[filter] = vector<VMMethodHandler>();
        }
        exit_filters[filter].push_back(handler);
    }

    void RecordMethod(char *file)
    {
        recording_folder = file;
        int error = FileTool::Start();
        if (!_recordMethodStarted)
        {
            jvmtiEventCallbacks callbacks;
            memset(&callbacks, 0, sizeof(callbacks));
            callbacks.MethodEntry = &_HandleMethodEntry;
            callbacks.MethodExit = &_HandleMethodExit;
            jvmtiError error;
            error = Global::global_vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
            Exception::HandleException(error);
            error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
            Exception::HandleException(error);
            error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, 0);
            Exception::HandleException(error);
            _recordMethodStarted = 1;
        }
    }

    void _MethodFrameHandler(VMModel::StackFrame **frame, jint size)
    {
        for (int i = 0; i < size; i++)
        {
            Logger::i("VMMethodService", frame[i]->vm_method->name);
        }
    }

    void _TestMethodFrameHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        VMFrameService::GetCurrentMethodFrame(thread, _MethodFrameHandler);
    }

    void TestMethodFrame()
    {
        Logger::d("MethodService", "TestMethodFrame");
        AddEntryFilter("firstMethod", _TestMethodFrameHandler);
        if (!_recordMethodStarted)
        {
            jvmtiEventCallbacks callbacks;
            memset(&callbacks, 0, sizeof(callbacks));
            callbacks.MethodEntry = &_HandleMethodEntry;
            jvmtiError error;
            error = Global::global_vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
            Exception::HandleException(error);
            error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
            Exception::HandleException(error);
            _recordMethodStarted = 1;
        }
    }

    void Release()
    {
        FileTool::Stop();
        VMModel::DeallocateMethodTask(task);
    }
}
