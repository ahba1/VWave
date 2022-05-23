#include "../service_header/vm_model.hpp"

#include "../global.hpp"

namespace ParamWatch
{
    void SetWatchField(char *method_name, char *param_name, char *type);

    void OnWatchField(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);
}