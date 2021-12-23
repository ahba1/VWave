#include "vm_service_manager.hpp"

class VMMethodService: VMService {
    VMMethodService(jvmtiEnv *vm_env): VMService(vm_env) {}

    string GetServiceName() {
        return "ThreadService";
    }

    void RegisterEventHandler();

    void AddFilter();
};

class MethodFilter {
    virtual bool filter();
};
