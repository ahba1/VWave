#pragma once
#include <map>
#include <string>
#include <string.h>
#include <jvmti.h>
#include "base/service/vm_service.hpp"
#include "thread_service/vm_thread_service.cpp"
#include "method_service/vm_method_service.hpp"
#include "base/error/vm_error.hpp"

typedef map<string, VMService*> VMSERVICE_MAP;

VMSERVICE_MAP service_map;

namespace VWaveService {
    void Init(jvmtiEnv *vm_env);

    void Destroyed();

    void CheckException(jvmtiError error) {
        if (error != JVMTI_ERROR_NONE) {
            throw VMError(&error);
        }
    }
}