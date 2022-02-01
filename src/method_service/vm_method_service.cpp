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

        VMModel::PrintMethod(method);
        vm_env->Deallocate(reinterpret_cast<unsigned char*>(method));
    }

    void HandleMethodTrace(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID) {
        jvmtiError error;
        jvmtiFrameInfo *frame_buffer;
        //VMModel::Method **methods;
        VMModel::Method *method;
        jint *count_ptr;
        //to-do, get depth from global config
        error = vm_env->GetStackTrace(thread, 0, 10, frame_buffer, count_ptr);
        //error = vm_env->Allocate(sizeof(VMModel::Method*), reinterpret_cast<unsigned char**>(methods));
        VMException::CheckException(error);
        for (int i = 0;i < *count_ptr;i++) {
            error = vm_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char**>(&method));
            VMException::CheckException(error);
            VMModel::MapJMethod(vm_env, methodID, method);

            VMModel::PrintMethod(method);
            vm_env->Deallocate(reinterpret_cast<unsigned char*>(method));
        }
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

void VMMethodService::AddFilter(char *filter, VMMethodHandler handler = DefVMMethodHandler) {
    filters[regex(filter)] = handler;
}

void VMMethodService::GetMethodTrace(char *methodName) {
    AddFilter(methodName, &_VMMethodService::HandleMethodTrace);
}