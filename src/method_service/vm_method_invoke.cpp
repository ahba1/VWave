
#include "vm_method_invoke.hpp"
#include "vm_jni_helper.hpp"

namespace MethodInvoke
{
    VMModel::MethodInvokeTask *task;

    jobject GetThis(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        jint entry_count;
        jvmtiLocalVariableEntry *table;
        jobject ret;
        error = vm_env->GetLocalVariableTable(method->meta->_id, &entry_count, &table);
        if (error == JVMTI_ERROR_ABSENT_INFORMATION)
        {
            Exception::HandleException(error, "recompile source file with -g");
        }
        else 
        {
            cout << "???" <<endl;
            Exception::HandleException(error);
        }
        for (int i=0;i<entry_count;i++)
        {
            if (!strcmp(table[i].name, "this"))
            {
                jvmtiError error = vm_env->GetLocalObject(thread, 0, table[i].slot, &ret);
                Exception::HandleException(error);
                break;
            }
        }
        for (int i=0;i<entry_count;i++)
        {
            error = vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(table[i].name));
            Exception::HandleException(error);
            error = vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(table[i].signature));
            Exception::HandleException(error);
            error = vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(table[i].generic_signature));
            Exception::HandleException(error);
        }
        error = vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(table));
        Exception::HandleException(error);
        return ret;
    }

    char *GetMethodSign()
    {
        return "()V";
    }

    void SetInvokeMethod(VMModel::MethodInvokeTask *_task)
    {
        task = _task;
    }

    void OnMethodInvoke(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jobject obj = GetThis(vm_env, jni, thread, method);
        VMJNIHelper::InvokeJavaMethod(obj, task->name, NULL, NULL);
    }
}