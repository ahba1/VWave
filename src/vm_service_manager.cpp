#include "vm_service_manager.hpp"

using namespace std;

namespace VWaveService {
    void Init(jvmtiEnv *vm_env) {
        service_map.insert(map<string, VMService*>::value_type("MethodService", new VMMethodService(vm_env)));
        service_map.insert(map<string, VMService*>::value_type("ThreadService", new VMThreadService(vm_env)));
    }

    void Destroyed() {
        map<string, VMService*>::iterator it;
        map<string, VMService*>::iterator it_end;
        it = service_map.begin();
        it_end = service_map.end();

        while (it != it_end)
        {
            delete it->second;
            it++;
        }
    }   
}