#pragma once
#include <map>
#include <string>
#include <jvmti.h>
#include "service/vm_service.hpp"
#include "thread_service/vm_thread_service.cpp"

using namespace std;

typedef map<string, VMService*> VMSERVICE_MAP;

VMSERVICE_MAP service_map;

namespace VWaveService {
    void Init(jvmtiEnv *vm_env);

    void Destroyed();
}