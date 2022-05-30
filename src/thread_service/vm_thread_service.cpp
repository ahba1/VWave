#include <map>
#include <algorithm>
#include "../service_header/vm_thread_service.hpp"
#include "../service_header/vwave_core.hpp"

namespace _VMThreadService
{
    VMThreadService *service;

    void JNICALL OnThreadStart(jvmtiEnv *jvmti_env,
                               JNIEnv *jni_env,
                               jthread thread)
    {
        service->OnThreadStart(jvmti_env, jni_env, thread);
    }

    void JNICALL OnThreadEnd(jvmtiEnv *jvmti_env,
                             JNIEnv *jni_env,
                             jthread thread)
    {
        service->OnThreadEnd(jvmti_env, jni_env, thread);
    }

    void JNICALL OnMethodEntry(jvmtiEnv *jvmti_env,
                               JNIEnv *jni_env,
                               jthread thread,
                               jmethodID methodID)
    {
        char *name;
        jvmtiError error = jvmti_env->GetMethodName(methodID, &name, NULL, NULL);
        Exception::HandleException(error);
        if (!strcmp(name, "interrupt") || !strcmp(name, "join") || !strcmp(name, "resume") || !strcmp(name, "suspend") || !strcmp(name, "sleep") || !strcmp(name, "yield") || !strcmp(name, "stop"))
        {
            jobject ret;
            jvmtiError error = jvmti_env->GetLocalObject(thread, 0, 0, &ret);
            if (error == JVMTI_ERROR_WRONG_PHASE || error == JVMTI_ERROR_OPAQUE_FRAME)
            {
                return;
            }
            Exception::HandleException(error);
            if (typeid(ret) == typeid(jthread))
            {
                service->OnExplicitChange(jvmti_env, jni_env, thread, (jthread)ret, name);
            }
        }
        else if (!strcmp(name, "wait"))
        {
            jobject ret;
            jvmtiError error = jvmti_env->GetLocalObject(thread, 0, 0, &ret);
            if (error == JVMTI_ERROR_WRONG_PHASE || error == JVMTI_ERROR_OPAQUE_FRAME)
            {

                return;
            }
            Exception::HandleException(error);
            service->OnImplicitChangeStart(jvmti_env, jni_env, thread, ret, name);
        }
        if (!strcmp(name, "notify") || !strcmp(name, "notifyAll"))
        {
            service->OnImplicitChangeStart(jvmti_env, jni_env, thread, thread, name);
        }
    }

    void JNICALL OnMethodExit(jvmtiEnv *jvmti_env,
                              JNIEnv *jni_env,
                              jthread thread,
                              jmethodID method,
                              jboolean was_popped_by_exception,
                              jvalue return_value)
    {
        char *name;
        jvmtiError error = jvmti_env->GetMethodName(method, &name, NULL, NULL);
        Exception::HandleException(error);
        if (!strcmp(name, "notify") || !strcmp(name, "notifyAll"))
        {
            service->OnImplicitChangeEnd(jvmti_env, jni_env, thread, thread, name);
        }
    }

    void GenerateExplicitChangeInfo(
        VMThread *invoker,
        VMThread *invokee,
        const char *method,
        const char *invoker_before_state,
        const char *invoker_after_state,
        const char *invokee_before_state,
        const char *invokee_after_state,
        char **ret)
    {
        if (invoker->name == NULL || invokee->name == NULL)
        {
            return;
        }
        StringTool::Concat(ret, {"<",
                                 invoker->name,
                                 " ",
                                 method,
                                 " ",
                                 invokee->name,
                                 " ",
                                 invoker_before_state,
                                 "->",
                                 invoker_after_state,
                                 " ",
                                 invokee_before_state,
                                 "->",
                                 invokee_after_state,
                                 ">"});
    }

    void GenerateImplicitChangeInfo(VMThread *invoker,
                                    jint invokee,
                                    const char *method,
                                    const char *invoker_before_state,
                                    const char *invoker_after_state,
                                    char **ret)
    {
        const char *invokee_ch = to_string(invokee).c_str();
        StringTool::Concat(ret, {"<",
                                 invoker->name,
                                 " ",
                                 method,
                                 " ",
                                 invokee_ch,
                                 " ",
                                 invoker_before_state,
                                 "->",
                                 invoker_after_state,
                                 ">"});
    }

    const char *GetStateDesc(int state)
    {
        switch (state)
        {
        case 0x01:
            return "NEW";
        case 0x02:
            return "RUNNABLE";
        case 0x04:
            return "RUNNING";
        case 0x08:
            return "BLOCKED";
        case 0x10:
            return "TERMINATED";
        default:
            return "UNKNOWN";
        }
    }

    uint8_t ConvertFromMethod(const char *method)
    {
        if (strcmp(method, "interrupt"))
        {
            return 0x08;
        }
        if (strcmp(method, "join"))
        {
            return 0x08;
        }
        if (strcmp(method, "resume"))
        {
            return 0x02;
        }
        if (strcmp(method, "suspend"))
        {
            return 0x08;
        }
        if (strcmp(method, "sleep"))
        {
            return 0x08;
        }
        if (strcmp(method, "yield"))
        {
            return 0x08;
        }
        if (strcmp(method, "stop"))
        {
            return 0x10;
        }
        if (strcmp(method, "wait"))
        {
            return 0x08;
        }
    }

    uint8_t ConvertFromOriginState(jint state)
    {
        switch (state)
        {
        case JVMTI_JAVA_LANG_THREAD_STATE_NEW:
        case JVMTI_THREAD_STATE_ALIVE:
            return 0x01;
        case JVMTI_THREAD_STATE_TERMINATED:
            return 0x10;
        case JVMTI_THREAD_STATE_RUNNABLE:
        case JVMTI_JAVA_LANG_THREAD_STATE_RUNNABLE:
        case 0x400005:
            return 0x02;
        case JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER:
        case JVMTI_THREAD_STATE_WAITING:
        case JVMTI_THREAD_STATE_WAITING_INDEFINITELY:
        case JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT:
        case JVMTI_THREAD_STATE_SLEEPING:
        case JVMTI_THREAD_STATE_IN_OBJECT_WAIT:
        case JVMTI_THREAD_STATE_PARKED:
        case JVMTI_THREAD_STATE_SUSPENDED:
        case JVMTI_THREAD_STATE_INTERRUPTED:
        case JVMTI_JAVA_LANG_THREAD_STATE_BLOCKED:
        case JVMTI_JAVA_LANG_THREAD_STATE_WAITING:
        case JVMTI_JAVA_LANG_THREAD_STATE_TIMED_WAITING:
        case 0x191:
            return 0x04;
        default:
            cout << state << endl;
            return 0;
        }
    }

    bool Compare(jobject obj1, jobject obj2)
    {
        jint hash1;
        jint hash2;
        jvmtiError error = Global::global_vm_env->GetObjectHashCode(obj1, &hash1);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetObjectHashCode(obj2, &hash2);
        Exception::HandleException(error);
        return hash1 > hash2;
    }

    map<jint, pair<jthread *, int>> waiters;

    void UpdateWaiters(jobject obj, jthread *new_waiters, int size)
    {
        if (size == 0)
        {
            return;
        }
        jvmtiError error;

        jthread *new_ptr;
        error = Global::global_vm_env->Allocate(sizeof(jthread) * size, reinterpret_cast<Global::memory_alloc_ptr>(&new_ptr));
        Exception::HandleException(error);

        jint hash;
        error = Global::global_vm_env->GetObjectHashCode(obj, &hash);
        Exception::HandleException(error);

        for (int i = 0; i < size; i++)
        {
            new_ptr[i] = new_waiters[i];
        }
        if (waiters.count(hash) != 0)
        {
            error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(waiters[hash].first));
            Exception::HandleException(error);
        }
        waiters[hash] = make_pair(new_ptr, size);
    }

    bool isSame(jobject obj1, jobject obj2)
    {
        jint hash1;
        jint hash2;
        jvmtiError error = Global::global_vm_env->GetObjectHashCode(obj1, &hash1);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetObjectHashCode(obj2, &hash2);
        Exception::HandleException(error);
        return hash1 == hash2;
    }

    void FindDiff(jthread *first, int size_1, jthread *second, int size_2, jthread **res, int *res_size)
    {
        jvmtiError error;
        int max = size_1 > size_2 ? size_1 : size_2;
        int min = size_1 < size_2 ? size_1 : size_2;
        error = Global::global_vm_env->Allocate(max * sizeof(jthread), reinterpret_cast<Global::memory_alloc_ptr>(res));
        Exception::HandleException(error);
        sort(first, first + size_1, Compare);
        sort(second, second + size_2, Compare);
        int _res_size = 0;
        int index1 = 0;
        int index2 = 0;
        while (index1 < size_1 && index2 < size_2)
        {
            if (isSame(first[index1], second[index2]))
            {
                index1++;
                index2++;
                continue;
            }
            if (Compare(first[index1], second[index2]))
            {
                (*res)[_res_size++] = second[index2];
            }
            else
            {
                (*res)[_res_size++] = first[index1];
            }
        }
        for (; index1 < size_1; index1++)
        {
            (*res)[_res_size++] = first[index1];
        }
        for (; index2 < size_2; index2++)
        {
            (*res)[_res_size++] = second[index2];
        }
        *res_size = _res_size;
    }

    vector<jint> dead_threads;

    bool IsDead(jthread thr)
    {
        jint hash;
        jvmtiError error = Global::global_vm_env->GetObjectHashCode(thr, &hash);
        return count(dead_threads.begin(), dead_threads.end(), hash);
    }

    int count = 10;
}

VMServiceDesc *VMThreadResolver::Resolve(char *file)
{
    return NULL;
}

VMThreadService::VMThreadService()
{
    resolver = VMThreadResolver();
}

VMThreadService::~VMThreadService()
{
    for (auto t : threads)
    {
        // t->Println(std::cout);
    }
    StringTool::DellocateChString(_target);
    for (auto i : _VMThreadService::waiters)
    {
        jvmtiError error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(i.second.first));
        Exception::HandleException(error);
        i.second.second = 0;
    }
    _VMThreadService::waiters.clear();
}

void VMThreadService::Init(uint8_t phase)
{
    jvmtiCapabilities caps;
    jvmtiError error;
    error = Global::global_vm_env->GetCapabilities(&caps);
    Exception::HandleException(error);
    caps.can_get_current_thread_cpu_time = 1;
    caps.can_get_thread_cpu_time = 1;
    caps.can_access_local_variables = 1;
    caps.can_generate_method_exit_events = 1;
    caps.can_generate_method_entry_events = 1;
    caps.can_get_owned_monitor_info = 1;
    caps.can_get_monitor_info = 1;
    error = Global::global_vm_env->AddCapabilities(&caps);
    Exception::HandleException(error);
    threads = vector<VMThread *>();
    _VMThreadService::service = this;
}

void VMThreadService::Invoke(char *target)
{
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ThreadStart = &_VMThreadService::OnThreadStart;
    callbacks.ThreadEnd = &_VMThreadService::OnThreadEnd;
    callbacks.MethodEntry = &_VMThreadService::OnMethodEntry;
    callbacks.MethodExit = &_VMThreadService::OnMethodExit;
    jvmtiError error;
    error = Global::global_vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    Exception::HandleException(error);
    error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, 0);
    Exception::HandleException(error);
    error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    Exception::HandleException(error);
    error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, 0);
    Exception::HandleException(error);
    error = Global::global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, 0);
    Exception::HandleException(error);
    StringTool::Copy(&_target, target);
}

void VMThreadService::OnPhaseChaged(uint8_t phase)
{
}

void VMThreadService::OnThreadStart(jvmtiEnv *jvmti_env,
                                    JNIEnv *jni_env,
                                    jthread thread)
{
    VMThread *prior = new VMThread();
    prior->Map(thread);
    prior->state = 0x01;
    threads.push_back(prior);
    // ofstream ofs(_target, ios::app);
    // ofs << "<" << prior->name << ", " << prior->priority << ", " << prior->is_daemon << ", " << (int)prior->state << ">";
    // ofs.close();
}

void VMThreadService::OnThreadEnd(jvmtiEnv *jvmti_env,
                                  JNIEnv *jni_env,
                                  jthread thread)
{
    jint hash;
    jvmtiError error = jvmti_env->GetObjectHashCode(thread, &hash);
    _VMThreadService::dead_threads.push_back(hash);  
    // ofstream ofs(_target, ios::app);
    // ofs << "<" << prior->name << ", " << prior->priority << ", " << prior->is_daemon << ", " << (int)prior->state << ">";
    // ofs.close();
    // for (auto waiter : _VMThreadService::waiters)
    // {
    //     for (int i=0;i<waiter.second.second;i++)
    //     {
    //         if (_VMThreadService::isSame(thread, waiter.second.first[i]))
    //         {
    //             while ((i + 1)<waiter.second.second)
    //             {
    //                 waiter.second.first[i] = waiter.second.first[i+1];
    //             }
    //             break;
    //         }
    //     }
        
    // }
}

void VMThreadService::OnExplicitChange(jvmtiEnv *jvmti_env,
                                       JNIEnv *jni_env,
                                       jthread invoker,
                                       jthread invokee,
                                       char *method)
{
    VMThread vm_invoker;
    vm_invoker.Map(invoker);
    VMThread vm_invokee;
    vm_invokee.Map(invokee);
    char *des;

    // invoker : before state is always RUNNING; after state is due to method
    const char *invoker_before_state = _VMThreadService::GetStateDesc(0x04);
    const char *invoker_after_state = _VMThreadService::GetStateDesc(_VMThreadService::ConvertFromMethod(method));

    // invokee : get state through GetThreadState
    jint state;
    jvmtiError error = jvmti_env->GetThreadState(invokee, &state);
    Exception::HandleException(error);
    const char *invokee_state = _VMThreadService::GetStateDesc(_VMThreadService::ConvertFromOriginState(state));
    _VMThreadService::GenerateExplicitChangeInfo(&vm_invoker,
                                                 &vm_invokee,
                                                 method,
                                                 invoker_before_state,
                                                 invoker_after_state,
                                                 invokee_state,
                                                 invokee_state,
                                                 &des);
    ofstream ofs(_target, ios::app);
    ofs << des << endl;
    ofs.close();
    StringTool::DellocateChString(des);
}

void VMThreadService::OnImplicitChangeStart(jvmtiEnv *jvmti_env,
                                            JNIEnv *jni_env,
                                            jthread invoker,
                                            jobject invokee,
                                            char *method)
{
    char *des;
    VMThread vm_invoker;
    vm_invoker.Map(invoker);
    jint invokee_hash;
    jvmtiError error = jvmti_env->GetObjectHashCode(invokee, &invokee_hash);
    Exception::HandleException(error);
    const char *invoker_before_state = _VMThreadService::GetStateDesc(0x04);
    const char *invoker_after_state = _VMThreadService::GetStateDesc(0x08);

    if (!strcmp("wait", method))
    {
        _VMThreadService::GenerateImplicitChangeInfo(&vm_invoker,
                                                     invokee_hash,
                                                     "wait",
                                                     invoker_before_state,
                                                     invoker_after_state, &des);

        ofstream ofs(_target, ios::app);
        ofs << des << endl;
        ofs.close();
        StringTool::DellocateChString(des);
    }
    else if (!strcmp("notify", method) || !strcmp("notifyAll", method))
    {
        jvmtiError error;
        jint owned_monitor_count = 0;
        jobject *monitors;
        error = jvmti_env->GetOwnedMonitorInfo(invoker, &owned_monitor_count, &monitors);
        if (error == JVMTI_ERROR_WRONG_PHASE)
        {
            return;
        }
        Exception::HandleException(error);
        for (int i = 0; i < owned_monitor_count; i++)
        {
            jvmtiMonitorUsage usage;
            error = jvmti_env->GetObjectMonitorUsage(monitors[i], &usage);
            Exception::HandleException(error);

            _VMThreadService::UpdateWaiters(monitors[i], usage.notify_waiters, usage.notify_waiter_count);
            error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(usage.waiters));
            Exception::HandleException(error);
            error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(usage.notify_waiters));
            Exception::HandleException(error);
        }
        error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(monitors));
        Exception::HandleException(error);
    }
}

void VMThreadService::OnImplicitChangeEnd(jvmtiEnv *jvmti_env,
                                          JNIEnv *jni_env,
                                          jthread invoker,
                                          jobject invokee,
                                          char *method)
{
    if (!strcmp("notify", method) || !strcmp("notifyAll", method))
    {
        jvmtiError error;
        jint owned_monitor_count = 0;
        jobject *monitors;
        error = jvmti_env->GetOwnedMonitorInfo(invoker, &owned_monitor_count, &monitors);
        if (error == JVMTI_ERROR_WRONG_PHASE)
        {
            return;
        }
        if (_VMThreadService::count <= 0)
        {
            return;
        }
        for (int i = 0; i < owned_monitor_count; i++)
        {
            jvmtiMonitorUsage usage;
            jint hash;
            jthread *diff;
            int diff_size = 0;

            error = jvmti_env->GetObjectMonitorUsage(monitors[i], &usage);
            Exception::HandleException(error);

            error = jvmti_env->GetObjectHashCode(monitors[i], &hash);
            Exception::HandleException(error);

            jthread *entry = _VMThreadService::waiters[hash].first;
            int entry_size = _VMThreadService::waiters[hash].second;

            _VMThreadService::FindDiff(entry, entry_size, usage.notify_waiters, usage.notify_waiter_count, &diff, &diff_size);
            for (int i = 0; i < diff_size; i++)
            {
                if (_VMThreadService::IsDead(invoker) || _VMThreadService::IsDead(diff[i]))
                {
                    continue;
                }
                VMThread vm_invoker;
                vm_invoker.Map(invoker);
                VMThread vm_invokee;
                vm_invokee.Map(diff[i]);
                char *des;
                const char *invoker_before_state = _VMThreadService::GetStateDesc(0x04);
                jint invoker_state;
                jvmtiError error = jvmti_env->GetThreadState(invoker, &invoker_state);
                Exception::HandleException(error);
                const char *invoker_after_state = _VMThreadService::GetStateDesc(_VMThreadService::ConvertFromOriginState(invoker_state));

                // invokee : get state through GetThreadState
                // jint invokee_state;
                // error = jvmti_env->GetThreadState(vm_invokee.meta, &invokee_state);
                // Exception::HandleException(error);
                const char *invokee_befor_state = _VMThreadService::GetStateDesc(0x08);
                // const char *invokee_after_state = _VMThreadService::GetStateDesc(_VMThreadService::ConvertFromOriginState(invokee_state));
                const char *invokee_after_state = _VMThreadService::GetStateDesc(0x02);

                _VMThreadService::GenerateExplicitChangeInfo(&vm_invoker,
                                                             &vm_invokee,
                                                             method,
                                                             invoker_before_state,
                                                             invoker_after_state,
                                                             invokee_befor_state,
                                                             invokee_after_state,
                                                             &des);
                if (des != NULL)
                {
                    ofstream ofs(_target, ios::app);
                    ofs << des << endl;
                    ofs.close();
                    StringTool::DellocateChString(des);
                    _VMThreadService::count--;
                }
            }
            _VMThreadService::UpdateWaiters(monitors[i], usage.notify_waiters, usage.notify_waiter_count);
            error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(diff));
            Exception::HandleException(error);
            error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(usage.waiters));
            Exception::HandleException(error);
            error = jvmti_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(usage.notify_waiters));
            Exception::HandleException(error);
        }
    }
}

VMThreadResolver &VMThreadService::GetResolver()
{
    return resolver;
}