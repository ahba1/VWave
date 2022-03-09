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

namespace _VMMethodService
{
    map<char*, VMMethodHandler> entry_filters;
    map<char*, VMMethodHandler> exit_filters;
    ofstream fout;

    void JNICALL HandleMethodEntry(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID)
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

    void JNICALL HandleMethodExit(
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

    void DefaultVMMethodHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        VMModel::PrintJMethod(method);
    }

    //todo according to thread to create responding file
    //todo create another file to record thread switch
    void RecordVMMethodEntryHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        fout << "entry " << method->name << endl;
    }

    void RecordVMMethodExitHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        fout << "exit " << method->name <<endl;
    }
}

VMMethodService::VMMethodService(jvmtiEnv *vm_env) : VMService(vm_env)
{
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof(caps));
    caps.can_generate_method_entry_events = 1;
    jvmtiError e = vm_env->AddCapabilities(&caps);
    Exception::HandleException(e);
}

void VMMethodService::DispatchCMD(char *key, char *value)
{
    if (!strcmp(key, "method"))
    {
        if (value)
        {
            this->AddEntryFilter(value, _VMMethodService::DefaultVMMethodHandler);
            if (!_is_started) {
                RegisterNormalEventHandler();
                _is_started = !_is_started;
            }
        }
        return;
    }
    if (!strcmp(key, "reord"))
    {
        if (value)
        {
            this->GetMethodTrace(value);
        }
        if (!_is_started) {
            RegisterMethodTraceHandler();
            _is_started = !_is_started;
        }
        return;
    }
    
}

/**
 * @brief parse options
 *
 * format: x=x&x=x
 *      method: method name(support regex)
 *      record: record to which file
 * @param options
 */
void VMMethodService::ParseOptions(char **options, int option_size)
{
    for (int i = 0; i < option_size; i++)
    {
        int kv_size = 0;
        char **cmd_kv = split(options[i], _spilt_kv_token, _max_kv_size, &kv_size);
        if (kv_size = _max_kv_size)
        {
            std::cout << cmd_kv[0] << "=" << cmd_kv[1] << std::endl;
            DispatchCMD(cmd_kv[0], cmd_kv[1]);
        }
    }
}

char *VMMethodService::GetServiceName()
{
    return "MethodService";
}

void VMMethodService::RegisterNormalEventHandler()
{
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = &_VMMethodService::HandleMethodEntry;
    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    Exception::HandleException(error);
}

void VMMethodService::RegisterMethodTraceHandler()
{
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = &_VMMethodService::HandleMethodEntry;
    callbacks.MethodExit = &_VMMethodService::HandleMethodExit;
    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, 0);
    Exception::HandleException(error);
}

void VMMethodService::AddEntryFilter(char *filter, _VMMethodService::VMMethodHandler handler = _VMMethodService::DefaultVMMethodHandler)
{
    _VMMethodService::entry_filters[filter] = handler;
}

void VMMethodService::AddExitFilter(char *filter, _VMMethodService::VMMethodHandler handler = _VMMethodService::DefaultVMMethodHandler)
{
    _VMMethodService::exit_filters[filter] = handler;
}

void VMMethodService::GetMethodTrace(char *file)
{
    _VMMethodService::fout.open(file);
    AddEntryFilter("*", _VMMethodService::RecordVMMethodEntryHandler);
    AddExitFilter("*", _VMMethodService::RecordVMMethodExitHandler);
}

VMMethodService::~VMMethodService()
{
    _VMMethodService::fout.close();
}
