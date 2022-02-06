#pragma once
#include <jvmti.h>

class VMService {

protected:
    jvmtiEnv *vm_env;

public:
    VMService(jvmtiEnv *vm_env) {
        this->vm_env = vm_env;
    }

    ~VMService() {
        //this->OnDestroyed();
    }

    virtual char* GetServiceName() = 0;
        
    //release all the src held in this function
    //virtual void OnDestroyed();
};