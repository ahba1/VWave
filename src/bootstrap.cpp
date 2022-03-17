#include <iostream>
#include <map>

#include <jvmti.h>
#include <fastcommon/shared_func.h>

#include "global.hpp"
#include "base/vm_service.hpp"
#include "method_service/vm_method_service.hpp"
#include "thread_service/vm_thread_service.hpp"

namespace Bootstrap
{
    using namespace Global;

    char *_options;

    void Init(JavaVM *vm, char *options, void *reserved);
    void Destroyed();
    void PreParseOptions(char *options);
    void LoadService(char *service_name, char **options, int options_size);
    int CheckParams(const char *key_params, char *params);

    void JNICALL OnVMInit(jvmtiEnv *jvmti, JNIEnv *env, jthread thr)
    {
        Global::global_jni_env = env;
        Global::global_agent_thread = &thr;
        PreParseOptions(_options);
    }

    void Init(JavaVM *vm, char *options, void *reserved)
    {
        jvmtiError error;
        global_java_vm = vm;
        _options = options;
        vm->GetEnv(reinterpret_cast<void **>(&global_vm_env), JVMTI_VERSION_1_0);
        jvmtiCapabilities caps;
        memset(&caps, 0, sizeof(caps));
        caps.can_generate_method_entry_events = 1;
        caps.can_generate_method_exit_events = 1;
        jvmtiError e = global_vm_env->AddCapabilities(&caps);
        Exception::HandleException(e);
        jvmtiEventCallbacks callbacks;
        callbacks.VMInit = &OnVMInit;
        error = global_vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
        Exception::HandleException(error);
        error = global_vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, 0);
        Exception::HandleException(error);
        std::cout << "load successfully..." << std::endl;
    }

    void Destroyed()
    {
        std::cout << "delete" << std::endl;
        std::map<char *, VMService *>::iterator it;
        std::map<char *, VMService *>::iterator it_end;
        it = services.begin();
        it_end = services.end();

        while (it != it_end)
        {
            delete it->second;
            it++;
        }
    }

    void PreParseOptions(char *options)
    {
        std::cout << options << std::endl;
        int option_size = 0;
        char **res = split(options, _spilt_token, _max_options_size, &option_size);
        if (option_size > 0)
        {
            int kv_size = 0;
            char **cmd_kv = split(res[0], _spilt_kv_token, _max_kv_size, &kv_size);
            char *key = cmd_kv[0];
            char *value = cmd_kv[1];
            if (CheckParams(_param_1, key))
            {
                std::cout << "unsupport parameter?: ";
                std::cout << key << std::endl;
                return;
            }
            Bootstrap::LoadService(value, &res[1], option_size - 1);
            freeSplit(cmd_kv);
        }
        freeSplit(res);
    }

    void LoadService(char *service_name, char **options, int options_size)
    {
        std::cout << service_name << std::endl;
        std::cout << *options << std::endl;
        if (!strcmp(service_name, "method"))
        {
            VMMethodService *method_service = new VMMethodService(global_vm_env);
            services[service_name] = method_service;
            method_service->ParseOptions(options, options_size);
            return;
        }
        if (!strcmp(service_name, "thread"))
        {
            VMThreadService *thread_service = new VMThreadService(global_vm_env);
            services[service_name] = thread_service;
            thread_service->ParseOptions(options, options_size);
            return;
        }
        if (!strcmp(service_name, "thread_method"))
        {
            VMThreadService *thread_service = new VMThreadService(global_vm_env);
            services["thread"] = thread_service;
            VMMethodService *method_service = new VMMethodService(global_vm_env);
            services["method"] = method_service;
            thread_service->ParseOptions(options, options_size);
            return;
        }
        std::cout << "unsupport parameters: ";
        std::cout << service_name << std::endl;
    }

    int CheckParams(const char *key_params, char *params)
    {
        return strcmp(key_params, params);
    }
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
    try
    {
        Bootstrap::Init(vm, options, reserved);
    }
    catch (jvmtiError &e)
    {
        std::cout << e << "\n";
        return JNI_ERR;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << "\n";
        return JNI_ERR;
    }

    return JNI_OK;
}

JNIEXPORT jint JNICALL
Agent_OnLoad_L(JavaVM *vm, char *options, void *reserved)
{
    return Agent_OnLoad(vm, options, reserved);
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *vm)
{
    Bootstrap::Destroyed();
    std::cout << "destroyed\n";
}

JNIEXPORT void JNICALL
Agent_OnUnload_L(JavaVM *vm)
{
    Agent_OnUnload(vm);
}