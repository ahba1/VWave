#include "global.hpp"

namespace Global
{
    int AllocateJNIEnv(JNIEnv **env)
    {
        if (global_java_vm == NULL)
        {
            return ERROR_JAVA_VM_NULL;
        }
        int status = global_java_vm->GetEnv(reinterpret_cast<void**>(env), JNI_VERSION_1_8);
        if (status == JNI_EDETACHED || *env == NULL)
        {
            status = global_java_vm->AttachCurrentThread(reinterpret_cast<void**>(env), NULL);
            if (status < 0)
            {
                *env = NULL;
            }
        }
        return ERROR_NONE;
    }

    int DeallocateJNIEnv(JNIEnv *env)
    {
        return global_java_vm->DetachCurrentThread();
    }
}