#include <jni.h>

namespace VMJNIHelper
{

    void InvokeJavaMethod(jobject obj, char *desc, void** values, void **ret);

    int GetParamsLen(const char *desc);

    jmethodID GetMid(JNIEnv *jni, char *desc, jclass klazz);

    void GetRetType(char *desc, char **ret);

    void ConvertJValue(char *desc, void **values, jvalue **ret);
}