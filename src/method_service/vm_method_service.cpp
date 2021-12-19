#include "vm_service_manager.hpp"

class VMMethodService: VMService {
public:
    VMMethodService(jvmtiEnv *vm_env): VMService(vm_env) {

    }

    string GetName() {
        return "ThreadService";
    }

    void OnDestroyed() {}
};