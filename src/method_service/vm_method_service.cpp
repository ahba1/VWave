#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <stack>
#include <regex>
#include <chrono>
#include <sstream>

#include <shared_func.h>

#include "../service_header/vm_method_service.hpp"

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
    map<char*, VMMethodHandler> entry_filters;
    map<char*, VMMethodHandler> exit_filters;
    stack<VMModel::MethodFrame*> method_stack;

    char *recording_folder;
    int _recordMethodStarted = 0;
    const char *_cost_file = "./cost.txt";

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
            }
            VMModel::DellocateMethodFrame(mf);
        }
    }

    void _RecordVMMethodEntryHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        _SetMethodEntryTime(method->name);
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);
        
        char *file;
        StringTool::Concat(&file, {recording_folder, vm_thread.thread_name, ".txt"});

        VMModel::VMClazz *vm_clazz;
        error = Global::global_vm_env->Allocate(sizeof(VMModel::VMClazz), reinterpret_cast<unsigned char**>(&vm_clazz));
        VMModel::MapJClazz(method->meta->_clazz, &vm_clazz);

        char *content;
        StringTool::Concat(&content, {"enter ", vm_clazz->source_file, "::", method->name, "\n"});

        FileTool::Output(file, content, strlen(content));
        
        VMModel::DellcateClazz(vm_clazz);
        VMModel::DellocateThread(vm_env, &vm_thread);
    }

    void _RecordVMMethodExitHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        _ResolveMethodEntryTime(method->name);
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);

        char *file;
        StringTool::Concat(&file, {recording_folder, vm_thread.thread_name, ".txt"});

        VMModel::VMClazz *vm_clazz;
        error = Global::global_vm_env->Allocate(sizeof(VMModel::VMClazz), reinterpret_cast<unsigned char**>(&vm_clazz));
        VMModel::MapJClazz(method->meta->_clazz, &vm_clazz);

        char *content;
        StringTool::Concat(&content, {"exit ", vm_clazz->source_file, "::", method->name, "\n"});

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
        jvmtiError e = Global::global_vm_env->AddCapabilities(&caps);
        Exception::HandleException(e);
    }

    void _DispathCMD(char *key, char *value)
    {
        Logger::d("MethodService", key);
        Logger::d("MethodService", value);
        if (!strcmp(key, "record"))
        {
            if (value)
            {
                RecordMethod(value);
            }
        }
    }

    void _ParseOptions(char **options, int size)
    {
        for (int i = 0;i < size; i++)
        {
            int kv_size = 0;
            char **cmd_kv = split(
                options[i],
                _spilt_kv_token, 
                _max_kv_size, 
                &kv_size);
            if (kv_size == _max_kv_size)
            {
                _DispathCMD(cmd_kv[0], cmd_kv[1]);
            }
        }
    }

    void JNICALL _HandleMethodEntry(
        jvmtiEnv *vm_env,
        JNIEnv *jni,
        jthread thread, 
        jmethodID methodID)
    {
        jvmtiError error;
        VMModel::Method *method;
        VMModel::MapJMethod(methodID, &method);
        map<char *, VMMethodHandler>::iterator it;
        it = entry_filters.begin();
        while (it != entry_filters.end())
        {   
            if (regex_search(method->name, regex(it->first)))
            {
                it->second(vm_env, jni, thread, method);
            }
            it++;
        }
        VMModel::DellocateMethod(method);
    }

    void JNICALL _HandleMethodExit(
        jvmtiEnv *jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jmethodID methodID,
        jboolean was_popped_by_exception,
        jvalue return_value)
    {
        jvmtiError error;
        VMModel::Method *method;
        VMModel::MapJMethod(methodID, &method);
        map<char *, VMMethodHandler>::iterator it;
        it = exit_filters.begin();
        while (it != exit_filters.end())
        {   
            if (regex_search(method->name, regex(it->first)))
            {
                it->second(jvmti_env, jni_env, thread, method);
            }
            it++;
        }
        VMModel::DellocateMethod(method);
    }

    void Init(char **options, int option_size)
    {
        _AllocateCapabilities();
        _ParseOptions(options, option_size);
    }

    void DefaultVMMethodHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        VMModel::PrintJMethod(method);
    }
    
    void AddEntryFilter(char *filter, VMMethodHandler handler)
    {
        entry_filters[filter] = handler;
    }

    void AddExitFilter(char *filter, VMMethodHandler handler)
    {
        exit_filters[filter] = handler;
    }

    void RecordMethod(char *file)
    {
        recording_folder = file;
        int error = FileTool::Start();
        if (!error)
        {
            AddEntryFilter(".*", _RecordVMMethodEntryHandler);
            AddExitFilter(".*", _RecordVMMethodExitHandler);
        }
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

    void Release()
    {
        FileTool::Stop();
    }
}

