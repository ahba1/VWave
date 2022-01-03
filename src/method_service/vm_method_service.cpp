#include <iostream>
#include "vm_method_service.hpp"
#include "../base/include/vm_error.hpp"

namespace _VMMethodService {
    void JNICALL HandleMethodEntry(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID) {
        jvmtiError error;
        VMModel::Method *method;
        error = vm_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char**>(&method));
        VMException::CheckException(error);
        VMModel::MapJMethod(vm_env, methodID, method);

        //method->Println();
        vm_env->Deallocate(reinterpret_cast<unsigned char*>(method));
    }
}

VMMethodService::VMMethodService(jvmtiEnv *vm_env): VMService(vm_env) {}

void VMMethodService::RegisterEventHandler() {
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = &_VMMethodService::HandleMethodEntry;

    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    VMException::CheckException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    VMException::CheckException(error);
}

void VMMethodService::AddFilter(char *filter) {
    filters.insert(regex(filter));
}