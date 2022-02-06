#include <iostream>
#include <stdio.h>

#include "vm_method_service.hpp"
#include "../base/vm_model.hpp"

namespace _VMMethodService {
    void JNICALL HandleMethodEntry(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID) {
        jvmtiError error;
        VMModel::Method *method;
        error = vm_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char**>(&method));
        Exception::HandleException(error);
        VMModel::MapJMethod(vm_env, methodID, method);
        //VMModel::PrintMethod(method);
        error = vm_env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
        if (strcmp("firstMethod", method->name) == 0 || strcmp("secondMethod", method->name) == 0)
        {
            /* code */
            std::cout<<method->name<<std::endl;

        }
        
        vm_env->Deallocate(reinterpret_cast<unsigned char*>(method));
    }

    void JNICALL HandleMethodTrace(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID) {
        jvmtiError error;
        jvmtiFrameInfo *frame_buffer;
        //VMModel::Method **methods;
        VMModel::Method *method;
        jint *count_ptr;
        //to-do, get depth from global config
        error = vm_env->GetStackTrace(thread, 0, 10, frame_buffer, count_ptr);
        //error = vm_env->Allocate(sizeof(VMModel::Method*), reinterpret_cast<unsigned char**>(methods));
        Exception::HandleException(error);

        for (int i = 0;i < *count_ptr;i++) {
            error = vm_env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char**>(&method));
            Exception::HandleException(error);
            VMModel::MapJMethod(vm_env, methodID, method);

            //VMModel::PrintMethod(method);
            std::cout<<method->name;
            vm_env->Deallocate(reinterpret_cast<unsigned char*>(method));
        }
    }
}

VMMethodService::VMMethodService(jvmtiEnv *vm_env): VMService(vm_env) {
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof(caps));
    caps.can_generate_method_entry_events = 1;
    jvmtiError e = vm_env->AddCapabilities(&caps);
    Exception::HandleException(e);
}

void VMMethodService::RegisterEventHandler() {
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = &_VMMethodService::HandleMethodEntry;
    jvmtiError error;
    error = vm_env->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
    Exception::HandleException(error);
    error = vm_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
    Exception::HandleException(error);
}

void VMMethodService::AddFilter(char *filter, VMMethodHandler handler) {
    //filters[regex(filter)] = handler;
}

void VMMethodService::GetMethodTrace(char *methodName) {
    AddFilter(methodName, &_VMMethodService::HandleMethodTrace);
}