#include "vm_service_manager.hpp"

class VMThreadService: public VMService {
public:
    VMThreadService(jvmtiEnv *vm_env): VMService(vm_env) {

    }

    string GetName() {
        return "ThreadService";
    }

    void OnDestroyed() {}

}; 