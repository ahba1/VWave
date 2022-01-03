#include "vm_service_manager.hpp"

JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    BootStrap::StartService(vm, options, reserved);
}

JNIEXPORT void JNICALL 
Agent_OnUnload(JavaVM *vm) {
    VWaveService::Destroyed();
}

namespace BootStrap {

    VMException::ERROR_TYPE StartService(JavaVM *vm, char *options, void *reserved) {
        jvmtiEnv *vm_env;
        vm->GetEnv(reinterpret_cast<void**>(&vm_env), JVMTI_VERSION_1_0);
        VWaveService::Init(vm_env);
    }
}