#include "vm_model.hpp"
#include "../global.hpp"

namespace VMModel
{
    void MapJClazz(jclass klazz, VMClazz **clazz)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(VMClazz), reinterpret_cast<Global::memory_alloc_ptr>(clazz));
        Exception::HandleException(error);
        VMClazz *_clazz = *clazz;
        error = Global::global_vm_env->Allocate(sizeof(ClazzMeta), reinterpret_cast<Global::memory_alloc_ptr>(&_clazz->meta));
        Exception::HandleException(error);
        _clazz->meta->_clazz = klazz;
        error = Global::global_vm_env->GetSourceFileName(klazz, &_clazz->source_file);
        Exception::HandleException(error);
    }

    void DellcateClazz(VMClazz *clazz)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz->source_file));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz));
        Exception::HandleException(error);
    }

    void MapJMethod(jmethodID methodID, Method **method)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(Method), reinterpret_cast<Global::memory_alloc_ptr>(method));
        Exception::HandleException(error);
        Method *_method = *method;
        error = Global::global_vm_env->Allocate(sizeof(Meta), reinterpret_cast<Global::memory_alloc_ptr>(&_method->meta));
        Exception::HandleException(error);
        _method->meta->_id = methodID;
        error = Global::global_vm_env->GetMethodDeclaringClass(methodID, &_method->meta->_clazz);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetMethodName(methodID, &_method->name, &_method->signature, &_method->generic);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetMethodModifiers(methodID, &_method->access_flag);
        Exception::HandleException(error);
        error = Global::global_vm_env->IsMethodNative(methodID, &_method->is_native);
        Exception::HandleException(error);
    }

    void DellocateMethod(Method *method)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->signature));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->generic));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method));
    }
}