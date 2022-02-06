#include <iostream>
#include <map>

#include <jvmti.h>

#include "base/vm_service.hpp"
#include "method_service/vm_method_service.hpp"

namespace Bootstrap {
    std::map<char*, VMService*> services;
    std::streambuf *globalOuter = std::cout.rdbuf();

    void Init(JavaVM *vm, char *options, void *reserved) {
        jvmtiEnv *vm_env;
        vm->GetEnv(reinterpret_cast<void**>(&vm_env), JVMTI_VERSION_1_0);
        VMMethodService *method_Service = new VMMethodService(vm_env);
        //services[method_Service->GetServiceName()] = method_Service;
        std::cout<<"????\n";
        method_Service->RegisterEventHandler();
        std::cout<<"load successfully..."<<std::endl;
    }

    void Destroyed() {
        std::map<char*, VMService*>::iterator it;
        std::map<char*, VMService*>::iterator it_end;
        it = services.begin();
        it_end = services.end();

        while (it != it_end)
        {
            delete it->second;
            it++;
        }
    }
}

JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    try
    {
        Bootstrap::Init(vm, options, reserved);
    }
    catch(jvmtiError& e)
    {
        std::cout<<e<<"\n";
        return JNI_ERR;
    }
    catch(std::exception& e) {
        std::cout<<e.what()<<"\n";
        return JNI_ERR;
    }
    
    return JNI_OK;
}

JNIEXPORT jint JNICALL
Agent_OnLoad_L(JavaVM *vm, char *options, void *reserved) {
    return Agent_OnLoad(vm, options, reserved);
}

JNIEXPORT void JNICALL 
Agent_OnUnload(JavaVM *vm) {
    Bootstrap::Destroyed();
    std::cout<<"destroyed\n";
}

JNIEXPORT void JNICALL 
Agent_OnUnload_L(JavaVM *vm) {
    Agent_OnUnload(vm);
}