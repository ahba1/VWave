#include <jvmti.h>

using namespace std;

class VMService {

protected:
    jvmtiEnv *vm_env;

public:
    VMService(jvmtiEnv *vm_env) {
        this->vm_env = vm_env;
    }

    ~VMService() {
        this->OnDestroyed();
    }

    virtual string GetServiceName();
    
    //release all the src held in this function
    virtual void OnDestroyed();
};