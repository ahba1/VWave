#include "vm_connection.hpp"
#include "vm_service_manager.hpp"

JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    jvmtiEnv *vm_env;
    vm->GetEnv((void**)&vm_env, JVMTI_VERSION_1_0);
    VWaveService::Init(vm_env);
}

JNIEXPORT void JNICALL 
Agent_OnUnload(JavaVM *vm) {
    VWaveService::Destroyed();
}