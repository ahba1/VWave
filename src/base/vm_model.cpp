#include "include/vm_model.hpp"
#include "include/vm_error.hpp"

namespace VMModel {
    bool ContainAccessFlag(AccessFlag source_flag, AccessFlag target) {
        return source_flag & target;
    }

    void MapJMethod(jvmtiEnv *env, jmethodID methodID, Method *method) {
        jvmtiError error;
        method->_methodID = methodID;
        error = env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
        VMException::CheckException(error);
        error = env->GetMethodModifiers(methodID, &method->access_flag);
        VMException::CheckException(error);
        error = env->IsMethodNative(methodID, &method->isNative);
        VMException::CheckException(error);
        Clazz *clazz = new Clazz();
        error = env->GetMethodDeclaringClass(methodID, &clazz->_klazz);
        VMException::CheckException(error);
        MapJClazz(env, clazz->_klazz, clazz);
    }

    void MapJClazz(jvmtiEnv *env, jclass klazz, Clazz *clazz) {
        jvmtiError error;
        clazz->_klazz = klazz;
        error = env->GetClassSignature(klazz, &clazz->signature, &clazz->generic);
        VMException::CheckException(error);
        error = env->GetSourceFileName(klazz, &clazz->name);
        VMException::CheckException(error);
        error = env->GetClassModifiers(klazz, &clazz->access_flag);
        VMException::CheckException(error);
        error = env->IsInterface(klazz, &clazz->isInterface);
        VMException::CheckException(error);
    }

    void DeallocateMethod(jvmtiEnv *env, Method *method) {
        env->Deallocate(reinterpret_cast<unsigned char*>(method->name));
        env->Deallocate(reinterpret_cast<unsigned char*>(method->generic));
        env->Deallocate(reinterpret_cast<unsigned char*>(method->signature));
        env->Deallocate(reinterpret_cast<unsigned char*>(method));
    }

    void MapDynamicMethod(jvmtiEnv *env, jmethodID methodID, DynamicMethod *dMethod) {
        jvmtiError error;
        VMModel::Method *method;
        error = env->Allocate(sizeof(VMModel::Method), reinterpret_cast<unsigned char**>(&method));
        VMException::CheckException(error);
        MapJMethod(env, methodID, method);
        dMethod->method = method;
        //to-do get the method stack trace info
    }

    void DeallocateDynamicMethod(jvmtiEnv *env, DynamicMethod *dMethod) {

    }

    void PrintMethod(Method* method, std::streambuf *target = std::cout.rdbuf()) {
        std::cout.rdbuf(target);
        if (method->generic) {
            std::cout<<method->generic<<" ";
        }
        if (method->name)
        {
            std::cout<<method->name<<std::endl;
        }
    
        std::cout.flush();
    }
    

    void JVMThread::Println(ThreadFormat *func, std::streambuf *target = std::cout.rdbuf()) {
            char *output = (*func)(info->name, state, info->priority, info->is_daemon);
            std::cout.rdbuf(target);
            std::cout<<output<<std::endl;
            std::cout.flush();
    }

    void MapJThread(jvmtiEnv *env, jthread _jthread, JVMThread *thread) {
        jvmtiError error;
        thread->_thread = _jthread;
        error = env->GetThreadInfo(_jthread, thread->info);
        VMException::CheckException(error);
        error = env->GetThreadState(_jthread, &thread->state);
        VMException::CheckException(error);
    }

    void DeallocateThread(jvmtiEnv *env, JVMThread *thread) {
        env->Deallocate(reinterpret_cast<unsigned char*>(thread->_thread));
        env->Deallocate(reinterpret_cast<unsigned char*>(thread->info));
        env->Deallocate(reinterpret_cast<unsigned char*>(thread));
    }
}