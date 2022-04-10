#include <iostream>

#include <jvmti.h>
#include <fastcommon/shared_func.h>

#include "global.hpp"
#include "service_header/vm_method_service.hpp"
#include "service_header/vm_class_service.hpp"
#include "service_header/vm_frame_service.hpp"

namespace Bootstrap
{
    using namespace Global;

    void Init(JavaVM *vm, char *options, void *reserved);
    void Destroyed();
    void PreParseOptions(char *options, bool isTest);
    void LoadService(char *service_name, char **options, int options_size);
    void Test(JavaVM *vm, char *options, void *reserved);
    void PreToolInit();
    int CheckParams(const char *key_params, char *params);

    typedef void (*SimplestMethod)();

    SimplestMethod *_DestroyMethods;
    int _DestroyMethodsLen = 0;

    void PreToolInit()
    {
        Logger::Init(Logger::Verbose | Logger::Debug | Logger::Info);
        Logger::i("Logger", "Init Success");
    }

    void Init(JavaVM *vm, char *options, void *reserved)
    {
        PreToolInit();
        global_java_vm = vm;
        vm->GetEnv(reinterpret_cast<void **>(&global_vm_env), JVMTI_VERSION_1_0);
        PreParseOptions(options, !strcmp(options, "test"));
        Logger::i("Bootstrap::Init", "load successfully");
    }

    void Destroyed()
    {
        // for (int i = 0;i < _DestroyMethodsLen;i++)
        // {
        //     _DestroyMethods[i]();
        // }
    }

    void PreParseOptions(char *options, bool isTest)
    {
        //std::cout << options << std::endl;
        if (isTest)
        {
            Test(Global::global_java_vm, options, NULL);
            return;
        }
        
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
                Logger::e("Parse", key);
                return;
            }
            Bootstrap::LoadService(value, &res[1], option_size - 1);
            freeSplit(cmd_kv);
        }
        freeSplit(res);
    }

    void LoadService(char *service_name, char **options, int options_size)
    {
        if (!strcmp(service_name, "method"))
        {
            Logger::i("LoadService", service_name);
            VMMethodService::Init(options, options_size);
            //VMClassService::Init(options, options_size);
            _DestroyMethodsLen++;
            return;
        }
        Logger::e("Load", service_name);
    }

    int CheckParams(const char *key_params, char *params)
    {
        return strcmp(key_params, params);
    }

    void Test(JavaVM *vm, char *options, void *reserved) 
    {
        //ThreadTool::Test();
        VMMethodService::Init(NULL, 0);
        VMFrameService::Init(NULL, 0);
        VMMethodService::TestMethodFrame();
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
    Logger::i("Destroy", "destroy successfully");
}

JNIEXPORT void JNICALL
Agent_OnUnload_L(JavaVM *vm)
{
    Agent_OnUnload(vm);
}