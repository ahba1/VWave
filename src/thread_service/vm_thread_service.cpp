#include <iostream>
#include <stdio.h>
#include <map>
#include <pthread.h>
#include <unistd.h> //attention, this header file is only used on Linux, in the future, this will adapt to Windows and Solaris

#include <shared_func.h>

#include "vm_thread_service.hpp"

namespace VMModel
{
    class VMThread
    {
    public:
        jthread _thread;
        jint _thread_state;
        char *thread_state;
        jvmtiThreadInfo *_info;
        char *thread_name;
        jint thread_priority;
        jboolean is_daemon;
        // temprorily not add group info and class loader info
    };

    void MapVMThread(jvmtiEnv *env, jthread thread, VMThread *vm_thread)
    {
        jvmtiError error;
        vm_thread->_thread = thread;
        error = env->GetThreadState(thread, &vm_thread->_thread_state);
        Exception::HandleException(error);
        MapThreadState(vm_thread->_thread_state, &vm_thread->thread_state);
        // get vm thread info, info includes its name, priority, is_damon, thread group, classloader
        MapLocalInfo(env, vm_thread);
    }

    void PrintVMThread(VMThread *vm_thread)
    {
        cout << vm_thread->thread_name << " state: " << vm_thread->thread_state << "\n";
    }

    void MapThreadState(int state, char **state_str)
    {
        switch (state)
        {
        case 0x0000:
            *state_str = "New";
            break;
        case 0x0001:
            *state_str = "Alive";
            break;
        case 0x0002:
            *state_str = "Terminated";
            break;
        case 0x0004:
            *state_str = "Runnable";
            break;
        case 0x0400:
            *state_str = "Waiting for a lock";
            break;
        case 0x0080:
            *state_str = "Waiting";
            break;
        case 0x0010:
            *state_str = "Waiting without a timeout";
            break;
        case 0x0020:
            *state_str = "Waiting with a timeout";
            break;
        case 0x0100:
            *state_str = "Object waiting";
            break;
        case 0x0200:
            *state_str = "Parked";
            break;
        case 0x100000:
            *state_str = "Suspended";
            break;
        case 0x200000:
            *state_str = "Interrupted";
            break;
        case 0x400000:
            *state_str = "Native thread running";
            break;
        default:
            *state_str = "Unknowns";
        }
    }

    void MapLocalInfo(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Allocate(sizeof(jvmtiThreadInfo), reinterpret_cast<unsigned char **>(&vm_thread->_info));
        Exception::HandleException(error);
        error = env->GetThreadInfo(vm_thread->_thread, vm_thread->_info);
        Exception::HandleException(error);
        vm_thread->thread_name = vm_thread->_info->name;
        vm_thread->thread_priority = vm_thread->_info->priority;
        vm_thread->is_daemon = vm_thread->_info->is_daemon;
    }

    void DellocateThread(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Deallocate(reinterpret_cast<unsigned char *>(vm_thread->_info));
    }
}

namespace _VMThreadService
{
    map<char *, ThreadWatcher> watchers;

    int monitor_thread_state = 0;

    const int QUERY_THREAD_SLEEP_TIME = 1; // TimeUnit: second

    void JNICALL OnThreadStart(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread)
    {
        OnThreadStateChange(jvmti_env, jni_env, thread);
    }

    void JNICALL OnThreadEnd(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread)
    {
        OnThreadStateChange(jvmti_env, jni_env, thread);
    }

    void DefaultThreadWatcher(jvmtiEnv *vm_env, JNIEnv *jni, VMModel::VMThread *vm_thread)
    {
        VMModel::PrintVMThread(vm_thread);
    }

    void OnThreadStateChange(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread)
    {
        jvmtiError error;
        VMModel::VMThread vm_thread;
        VMModel::MapVMThread(jvmti_env, thread, &vm_thread);
        if (watchers.find(vm_thread.thread_name) != watchers.end())
        {
            watchers[vm_thread.thread_name](jvmti_env, jni_env, &vm_thread);
        }
    }

    void *QueryThreadState(void *arg)
    {
        jint error;
        jvmtiError jerror;
        map<char *, int> states;
        for (map<char *, ThreadWatcher>::iterator it = watchers.begin(); it != watchers.end(); it++)
        {
            states[it->first] = -1;
        }
        error = Global::global_java_vm->AttachCurrentThread(reinterpret_cast<void **>(&Global::global_vm_env), NULL);
        Exception::HandleException(error);
        _VMThreadService::monitor_thread_state = 1;
        jint thread_count;
        jthread *threads_ptr;
        while (monitor_thread_state)
        {
            // sleep and check other thread state;
            jerror = Global::global_vm_env->GetAllThreads(&thread_count, &threads_ptr);
            if (error != JVMTI_ERROR_NONE)
            {
                break;
            }
            try
            {
                for (int i = 0; i < thread_count; i++)
                {
                    VMModel::VMThread vm_thread;
                    VMModel::MapVMThread(Global::global_vm_env, threads_ptr[i], &vm_thread);
                    if (states.find(vm_thread.thread_name) != states.end() && states[vm_thread.thread_name] != vm_thread._thread_state)
                    {
                        OnThreadStateChange(Global::global_vm_env, NULL, threads_ptr[i]);
                        states[vm_thread.thread_name] = vm_thread._thread_state;
                    }
                }
            }
            catch (jvmtiError e)
            {
                break;
            }
            sleep(QUERY_THREAD_SLEEP_TIME);
        }
        error = Global::global_java_vm->DetachCurrentThread();
        Exception::HandleException(error);
        Exception::HandleException(jerror);
    }

    void CreateJNIThread(pthread_t thread_t, VWaveThreadFunc func, void *args)
    {
        int error = pthread_create(&thread_t, NULL, func, args);
        Exception::HandleException(error);
    }
}

VMThreadService::VMThreadService(jvmtiEnv *env) : VMService(env)
{
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof(caps));
    caps.can_signal_thread = 1;
    jvmtiError e = env->AddCapabilities(&caps);
    Exception::HandleException(e);
}

void VMThreadService::DispatchCMD(char *key, char *value)
{
}

void VMThreadService::ParseOptions(char **options, int option_size)
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

char *VMThreadService::GetServiceName()
{
    return "ThreadService";
}

void VMThreadService::GetCurrentThreadInfo()
{
    jvmtiError error;
    jthread *_thread;
    VMModel::VMThread vm_thread;
    error = vm_env->GetCurrentThread(_thread);
    Exception::HandleException(error);
    VMModel::MapVMThread(vm_env, *_thread, &vm_thread);
    VMModel::PrintVMThread(&vm_thread);
    VMModel::DellocateThread(vm_env, &vm_thread);
}

void VMThreadService::MonitorThread(char *thread_name, _VMThreadService::ThreadWatcher watcher = _VMThreadService::DefaultThreadWatcher)
{
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ThreadStart = &_VMThreadService::OnThreadStart;
    callbacks.ThreadEnd = &_VMThreadService::OnThreadEnd;
    _VMThreadService::watchers[thread_name] = watcher;
    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, 0);
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, 0);
    Exception::HandleException(error);
}

void VMThreadService::StartMonitorThread()
{
    pthread_t thread_t;
    _VMThreadService::CreateJNIThread(thread_t, _VMThreadService::QueryThreadState, NULL);
}

void VMThreadService::EndMoitorThread()
{
    _VMThreadService::monitor_thread_state = 0;
}

VMThreadService::~VMThreadService()
{
    EndMoitorThread();
}