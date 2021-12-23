#include <iostream>
#include "vm_method_service.hpp"

namespace _VMMethodService {
    void JNICALL HandleMethodEntry(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID method) {
        try {
            jvmtiError error;
            jclass clazz;
            char *name;
            char *signature;

            //get class of method
            error = vm_env->GetMethodDeclaringClass(method, &clazz);
            VWaveService::CheckException(error);
            //get signature of class
            error = vm_env->GetClassSignature(clazz, &signature, 0);
            VWaveService::CheckException(error);
            //get name of method 
            error = vm_env->GetMethodName(method, &name, NULL, NULL);
            VWaveService::CheckException(error);
            std::cout << signature << " -> " << name << "(..)" << std::endl;

            //release memory
            error = vm_env->Deallocate(reinterpret_cast<unsigned char*>(name));
            VWaveService::CheckException(error);
            error = vm_env->Deallocate(reinterpret_cast<unsigned char*>(signature));
            VWaveService::CheckException(error);
        } catch (VMError e) {
            std::cout << "there is an error when handle method entry" << std::endl;
        }
    }
}

void VMMethodService::RegisterEventHandler() {
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = &_VMMethodService::HandleMethodEntry;

    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    VWaveService::CheckException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    VWaveService::CheckException(error);
}