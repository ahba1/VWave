#include "vm_model.hpp"

namespace VMModel {
    // bool ContainAccessFlag(AccessFlag source_flag, AccessFlag target) {
    //     return source_flag & target;
    // }

    // void MapJMethod(jvmtiEnv *env, jmethodID methodID, Method *method) {
    //     jvmtiError error;
    //     //Clazz *clazz;
    //     method->_methodID = methodID;
    //     error = env->GetMethodName(methodID, &method->name, &method->signature, &method->generic);
    //     Exception::HandleException(error);
    //     error = env->GetMethodModifiers(methodID, &method->access_flag);
    //     Exception::HandleException(error);
    //     error = env->IsMethodNative(methodID, &method->isNative);
    //     Exception::HandleException(error);
    //     //error = env->Allocate(sizeof(Clazz), reinterpret_cast<unsigned char**>(&clazz));
    //     Exception::HandleException(error);
    //     //error = env->GetMethodDeclaringClass(methodID, &clazz->_klazz);
    //     Exception::HandleException(error);
    //     //MapJClazz(env, clazz->_klazz, clazz);
    // }

    // void MapJClazz(jvmtiEnv *env, jclass klazz, Clazz *clazz) {
    //     jvmtiError error;
    //     clazz->_klazz = klazz;
    //     error = env->GetClassSignature(klazz, &clazz->signature, &clazz->generic);
    //     Exception::HandleException(error);
    //     error = env->GetSourceFileName(klazz, &clazz->name);
    //     std::cout<<error<<"\n";
    //     Exception::HandleException(error);
    //     error = env->GetClassModifiers(klazz, &clazz->access_flag);
    //     Exception::HandleException(error);
    //     error = env->IsInterface(klazz, &clazz->isInterface);
    //     Exception::HandleException(error);
    // }

    // void DeallocateMethod(jvmtiEnv *env, Method *method) {
    //     env->Deallocate(reinterpret_cast<unsigned char*>(method->name));
    //     env->Deallocate(reinterpret_cast<unsigned char*>(method->generic));
    //     env->Deallocate(reinterpret_cast<unsigned char*>(method->signature));
    //     env->Deallocate(reinterpret_cast<unsigned char*>(method));
    // }
}