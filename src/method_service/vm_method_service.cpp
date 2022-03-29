#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include <regex>

#include <shared_func.h>

#include "vm_method_service.hpp"

namespace VMModel
{

    class Method
    {
    public:
        jmethodID _methodID;
        char *name;
        char *signature;
        char *generic;
        jint access_flag;
        jboolean is_native;
    };

    void MapJMethod(jvmtiEnv *env, jmethodID methodID, Method *method)
    {
        jvmtiError error;
        method->_methodID = methodID;
        error = env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
        Exception::HandleException(error);
        error = env->GetMethodModifiers(methodID, &method->access_flag);
        Exception::HandleException(error);
        error = env->IsMethodNative(methodID, &method->is_native);
        Exception::HandleException(error);
    }

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
    char *recording_folder;
    int _recordMethodStarted = 0;

    void _RecordVMMethodEntryHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);

        char *file_suffix = ".txt";
        char *file;
        error = vm_env->Allocate(strlen(recording_folder) + strlen(vm_thread.thread_name) + strlen(file_suffix), reinterpret_cast<unsigned char**>(&file));
        Exception::HandleException(error);
        strcpy(file, recording_folder);
        strcat(file, vm_thread.thread_name);
        strcat(file, file_suffix);
        //cout << file <<endl;

        char *content_prefix = "enter ";
        char *content;
        error = vm_env->Allocate(strlen(content_prefix) + strlen(method->name) + 1, reinterpret_cast<unsigned char**>(&content));
        Exception::HandleException(error);
        strcpy(content, content_prefix);
        strcat(content, method->name);
        strcat(content, "\n");
        //cout << content;
        FileTool::Output(file, content, strlen(content));

        VMModel::DellocateThread(vm_env, &vm_thread);
    }

    void _RecordVMMethodExitHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(vm_env, thread, &vm_thread);
        char *file_suffix = ".txt";
        char *file;
        error = vm_env->Allocate(strlen(recording_folder) + strlen(vm_thread.thread_name) + strlen(file_suffix) + 1, reinterpret_cast<unsigned char**>(&file));
        Exception::HandleException(error);
        strcpy(file, recording_folder);
        strcat(file, vm_thread.thread_name);
        strcat(file, file_suffix);
        //cout << file <<endl;

        char *content_prefix = "exit ";
        char *content;
        error = vm_env->Allocate(strlen(content_prefix) + strlen(method->name) + 1, reinterpret_cast<unsigned char**>(&content));
        Exception::HandleException(error);
        strcpy(content, content_prefix);
        strcat(content, method->name);
        strcat(content, "\n");
        //cout << content;
        FileTool::Output(file, content, strlen(content));
        VMModel::DellocateThread(vm_env, &vm_thread);
    }

    void _AllocateCapabilities()
    {
        jvmtiCapabilities caps;
        Logger::i("VMMethodService", "AllocateCapabilities");
        memset(&caps, 0, sizeof(caps));
        caps.can_generate_method_entry_events = 1;
        caps.can_generate_method_exit_events = 1;
        jvmtiError e = Global::global_vm_env->AddCapabilities(&caps);
    }

    void _DispathCMD(char *key, char *value)
    {
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
        error = vm_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char **>(&method));
        Exception::HandleException(error);
        VMModel::MapJMethod(vm_env, methodID, method);
        error = vm_env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
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
        vm_env->Deallocate(reinterpret_cast<unsigned char *>(method));
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
        error = jvmti_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char **>(&method));
        Exception::HandleException(error);
        VMModel::MapJMethod(jvmti_env, methodID, method);
        error = jvmti_env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
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
        jvmti_env->Deallocate(reinterpret_cast<unsigned char *>(method));
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
            _recordMethodStarted = 1;
        }
        
    }

    void Release()
    {
        FileTool::Stop();
    }
}

