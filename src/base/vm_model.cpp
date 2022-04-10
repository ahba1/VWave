#include "../service_header/vm_model.hpp"
#include "../global.hpp"

namespace VMModel
{
    void MapVMThread(jvmtiEnv *env, jthread thread, VMThread *vm_thread)
    {
        jvmtiError error;
        vm_thread->_thread = thread;
        error = env->GetThreadState(thread, &vm_thread->_thread_state);
        Exception::HandleException(error);
        MapThreadState(vm_thread->_thread_state, &vm_thread->thread_state);
        // get vm thread info, info includes its name, priority, is_damon, thread group, classloader
        MapLocalInfo(env, vm_thread);
    }

    void PrintVMThread(VMThread *vm_thread)
    {
        cout << vm_thread->thread_name << " state: " << vm_thread->thread_state << "\n";
    }

    void MapThreadState(int state, char **state_str)
    {
        switch (state)
        {
        case 0x0000:
            *state_str = "New";
            break;
        case 0x0001:
            *state_str = "Alive";
            break;
        case 0x0002:
            *state_str = "Terminated";
            break;
        case 0x0004:
            *state_str = "Runnable";
            break;
        case 0x0400:
            *state_str = "Waiting for a lock";
            break;
        case 0x0080:
            *state_str = "Waiting";
            break;
        case 0x0010:
            *state_str = "Waiting without a timeout";
            break;
        case 0x0020:
            *state_str = "Waiting with a timeout";
            break;
        case 0x0100:
            *state_str = "Object waiting";
            break;
        case 0x0200:
            *state_str = "Parked";
            break;
        case 0x100000:
            *state_str = "Suspended";
            break;
        case 0x200000:
            *state_str = "Interrupted";
            break;
        case 0x400000:
            *state_str = "Native thread running";
            break;
        default:
            *state_str = "Unknowns";
        }
    }

    void MapLocalInfo(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Allocate(sizeof(jvmtiThreadInfo), reinterpret_cast<unsigned char **>(&vm_thread->_info));
        Exception::HandleException(error);
        error = env->GetThreadInfo(vm_thread->_thread, vm_thread->_info);
        Exception::HandleException(error);
        vm_thread->thread_name = vm_thread->_info->name;
        vm_thread->thread_priority = vm_thread->_info->priority;
        vm_thread->is_daemon = vm_thread->_info->is_daemon;
    }

    void DellocateThread(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Deallocate(reinterpret_cast<unsigned char *>(vm_thread->_info));
    }
    
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
        Exception::HandleException(error);
    }

    void CreateMethodFrame(MethodFrame **mf, const char *method)
    {
        using namespace std::chrono;
        jvmtiError error;
        time_point<high_resolution_clock> _time = high_resolution_clock::now();
        error = Global::global_vm_env->Allocate(sizeof(MethodFrame), reinterpret_cast<Global::memory_alloc_ptr>(mf));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(strlen(method) + 1, reinterpret_cast<Global::memory_alloc_ptr>(&(*mf)->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(sizeof(time_point<high_resolution_clock>), reinterpret_cast<Global::memory_alloc_ptr>(&(*mf)->tm));
        Exception::HandleException(error);
        strcpy((*mf)->name, method);
        *((*mf)->tm) = _time;
    }

    void DellocateMethodFrame(MethodFrame *mf)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf->tm));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf));
        Exception::HandleException(error);
    }

    void MapStackFrame(jvmtiFrameInfo *info, StackFrame **sf)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(StackFrame), reinterpret_cast<Global::memory_alloc_ptr>(sf));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(sizeof(StackFrameMeta), reinterpret_cast<Global::memory_alloc_ptr>(&(*sf)->meta));
        Exception::HandleException(error);
        MapJMethod(info->method, &(*sf)->vm_method);
        Logger::d("Map", (*sf)->vm_method->name);
    }

    void DellocateStackFrame(StackFrame *sf)
    {
        jvmtiError error;
        DellocateMethod(sf->vm_method);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(sf->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(sf));
        Exception::HandleException(error);
    }
}