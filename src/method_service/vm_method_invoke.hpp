#include "../service_header/vm_model.hpp"

#include "../global.hpp"

namespace MethodInvoke
{
    void SetInvokeMethod(VMModel::MethodInvokeTask *task);

    void OnMethodInvoke(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);
}