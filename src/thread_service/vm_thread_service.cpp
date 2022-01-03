#include "vm_thread_service.hpp"

VMThreadService::VMThreadService(jvmtiEnv *vm_env): VMService(vm_env){}

void VMThreadService::clearCachedThreads() {
    for (int i=0;i<threads.size();i++) {
        VMModel::DeallocateThread(vm_env, threads[i]);
    }
    vm_env->Deallocate(reinterpret_cast<unsigned char*>(_jthreads));
}

void VMThreadService::OnDestroyed() {
    clearCachedThreads();
}

void VMThreadService::GetAllThread() {
    jvmtiError error;
    jint size;
    clearCachedThreads();
    error = vm_env->GetAllThreads(&size, &_jthreads);
    for (int i=0;i<size;i++) {
        VMModel::JVMThread *t;
        error = vm_env->Allocate(sizeof(VMModel::JVMThread), reinterpret_cast<unsigned char**>(&t));
        VWaveService::CheckException(error);
        threads[i] = t;
    }
}

void VMThreadService::DumpThreadsInfo(VMModel::Format *format, std::streambuf *target = std::cout.rdbuf()) {
    for (int i=0;i<threads.size();i++) {
        threads[i]->Println(format, target);
    }
}