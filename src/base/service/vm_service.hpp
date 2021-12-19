#include <jvmti.h>
#include <string>

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

    virtual string getServiceName();
    
    //release all the src held in this function
    virtual void OnDestroyed();
}