#pragma once
#include <map>
#include <string>
#include <string.h>
#include <jvmti.h>

#include "base/service/vm_service.hpp"
#include "thread_service/vm_thread_service.hpp"
#include "method_service/vm_method_service.hpp"
#include "base/include/vm_error.hpp"

typedef map<string, VMService*> VMSERVICE_MAP;

VMSERVICE_MAP service_map;

namespace VWaveService {
    void Init(jvmtiEnv *vm_env);

    void Destroyed();

    void DumpSnapShot();
}