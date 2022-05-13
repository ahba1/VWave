#include <fastcommon/shared_func.h>
#include <string>

#include "vm_jni_helper.hpp"
#include "../service_header/vwave_core.hpp"
#include "../global.hpp"

namespace VMJNIHelper
{
    char **_SplitDescription(char *desc)
    {
        char **res;
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(char*) * 2, reinterpret_cast<Global::memory_alloc_ptr>(res));
        Exception::HandleException(error);
        res[0] = strtok(desc, "::");
        res[1] = strtok(NULL, "::");
        return res;
    }

    void InvokeJavaMethod(jobject obj, char *desc, void** values, void **ret)
    {
        char *_desc = NULL;
        StringTool::Copy(&_desc, desc);
        char *ret_type;
        GetRetType(_desc, &ret_type);
        JNIEnv *jni;
        Exception::HandleExternalException(Global::AllocateJNIEnv(&jni));
        jclass klazz = jni->GetObjectClass(obj);
        if (!strcmp(ret_type, "Z") || !strcmp(ret_type, "z"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jboolean), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jboolean*)*ret) = jni->CallBooleanMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        } 
        else if (!strcmp(ret_type, "B") || !strcmp(ret_type, "z"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jbyte), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jbyte*)*ret) = jni->CallByteMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "C") || !strcmp(ret_type, "c"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jchar), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jchar*)*ret) = jni->CallCharMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "L") || !strcmp(ret_type, "l"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jlong), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jlong*)*ret) = jni->CallLongMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "F") || !strcmp(ret_type, "f"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jfloat), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jfloat*)*ret) = jni->CallFloatMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "D") || !strcmp(ret_type, "d"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jdouble), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jdouble*)*ret) = jni->CallDoubleMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "S") || !strcmp(ret_type, "s"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jshort), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jshort*)*ret) = jni->CallShortMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "I") || !strcmp(ret_type, "i"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jint), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jint*)*ret) = jni->CallIntMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(ret_type, "V") || !strcmp(ret_type, "v"))
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            jni->CallVoidMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else 
        {
            jmethodID mid = GetMid(jni, _desc, klazz);
            Global::global_vm_env->Allocate(sizeof(jobject), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jobject*)*ret) = jni->CallObjectMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        StringTool::DellocateChString(ret_type);
        StringTool::DellocateChString(_desc);
    }

    int GetParamsLen(const char *types)
    {
        int len = 0;
        for (int i = 0; ; i++)
        {
            char ch = types[i];
            if (ch == '\0')
            {
                break;
            }
            if (ch == 'l' || ch == 'L')
            {
                while (types[i] != ';')
                {
                    i++;
                }
            }
        }
        return len;
    }

    jmethodID GetMid(JNIEnv *jni, char *desc, jclass klazz)
    {
        char **res = _SplitDescription(desc);
        char *method_name = strtok(res[1], "(");
        char *ch = strtok(NULL, "(");
        char *method_desc;
        StringTool::Concat(&method_desc, {"(", ch});
        jmethodID mid = jni->GetMethodID(klazz, method_name, method_desc);
        StringTool::DellocateChString(method_desc);
        Exception::HandleException(Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(res)));
        return mid;
    }

    void GetRetType(char *desc, char **ret)
    {
        char **res = _SplitDescription(desc);
        char *ret_type = strtok(res[1], ")");
        ret_type = strtok(NULL, ")");
        StringTool::Copy(ret, ret_type);
    }

    void ConvertJValue(char *desc, void **values, jvalue **ret)
    {
        // char **res = _SplitDescription(desc);
        // strtok(res[1], "(");
        // int values_ptr = 0;
        // void *_values = *values; 
        // char *types = strtok(NULL, ")");
        // //ret = Global::global_vm_env->Allocate(strlen(desc) * sizeof(jvalue), )
        // for (int i = 0; ; i++)
        // {
        //     char ch = types[i];
        //     if (ch == '\0')
        //     {
        //         break;
        //     }
        //     else 
        //     {
        //         switch (ch)
        //         {
        //         case 'z':
        //         case 'Z':
        //             jboolean b = (jboolean)(((uint8_t*)_values)[values_ptr++]);
        //             break;
        //         case 'b':
        //         case 'B':
        //             jbyte b = (jbyte)(((int8_t*)_values)[values_ptr++]);
        //             break;
        //         case 'c':
        //         case 'C':
        //             jchar b = (jchar)(((uint16_t*)_values)[values_ptr++]);
        //             break;
        //         case 'l':
        //         case 'L':
        //             jlong b = (jlong)(((int64_t*)_values)[values_ptr++]);   
        //             break;
        //         case 'f':
        //         case 'F':
        //             jfloat b = (jfloat)(((_Float32*)_values)[values_ptr++]);
        //             break;
        //         case 'd':
        //         case 'D':
        //             jdouble b = (jdouble)(((_Float64*)_values)[values_ptr++]);
        //             break;
        //         case 's':
        //         case 'S':
        //             jshort b = (jshort)(((int16_t*)_values)[values_ptr++]);
        //             break;
        //         case 'i':
        //         case 'I':
        //             jint b = (jint)(((int32_t*)_values)[values_ptr++]);
        //             break;
        //         case 'v':
        //         case 'V':
        //             break;
        //         default:
        //             break;
        //         }
        //     }
        // }
    }
}