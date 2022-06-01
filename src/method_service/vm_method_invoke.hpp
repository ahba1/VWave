#include "../service_header/vm_model.hpp"

#include "../global.hpp"

namespace MethodInvoke
{
    void SetInvokeMethod(char *method_name, VMModel::MethodInvokeTask *task);

    void OnMethodInvoke(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);

    void InvokeInstanceMethod(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jobject target, char *method, jvalue *params, jvalue *ret);

    void GetFullClassName(char *desc, char **ret);

    void GetMethodName(char *desc, char **ret);

    void GetMethodDesc(char *desc, char **ret);

    void GetMethodRetType(char *desc, char **ret);

    void ConvertToJValue(jvmtiEnv *vm_env,
                         JNIEnv *jni,
                         jthread thread,
                         VMModel::Method *method,
                         VMModel::MethodInvokeTask *task,
                         jvalue **params,
                         int *size);
}