#include <jni.h>

namespace VMJNIHelper
{

    void InvokeJavaMethod(jobject obj, char *desc, void** values, void **ret);

    int GetParamsLen(const char *desc);

    void ConvertJValue(char *desc, void **values, jvalue **ret);
}