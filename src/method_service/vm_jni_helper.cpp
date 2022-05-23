#include <fastcommon/shared_func.h>
#include <string>

#include "vm_jni_helper.hpp"
#include "../service_header/vwave_core.hpp"
#include "../global.hpp"

namespace VMJNIHelper
{
    char *_desc = NULL;
    char *_class = NULL;
    char *_method_name = NULL;
    char *_method_type = NULL;
    char *_ret_type = NULL;
    void _SplitDescription(char *desc)
    {
        char *temp_desc;
        StringTool::Copy(&temp_desc, desc);
        StringTool::Copy(&_desc, temp_desc);
        StringTool::Copy(&_class, strtok(temp_desc, "::"));
        char *name_type = strtok(NULL, "::");
        StringTool::Copy(&_method_name, strtok(name_type, "("));
        StringTool::Concat(&_method_type, {"(", strtok(NULL, "(")});

        char *temp;
        StringTool::Copy(&temp, _method_type);
        char *temp_strtok = temp;
        strtok(temp_strtok, ")");
        StringTool::Copy(&_ret_type, strtok(NULL, ")"));
        StringTool::DellocateChString(temp_strtok);
        StringTool::DellocateChString(temp_desc);
        Logger::i("MethodResolve", _desc);
        Logger::i("MethodResolve", _class);
        Logger::i("MethodResolve", _method_name);
        Logger::i("MethodResolve", _method_type);
        Logger::i("MethodResolve", _ret_type);
    }

    void InvokeJavaMethod(jobject obj, char *desc, void** values, void **ret)
    {
        _SplitDescription(desc);
        JNIEnv *jni;
        Exception::HandleExternalException(Global::AllocateJNIEnv(&jni));
        jclass klazz = jni->GetObjectClass(obj);
        jmethodID mid = jni->GetMethodID(klazz, _method_name, _method_type);
        if (!strcmp(_ret_type, "Z") || !strcmp(_ret_type, "z"))
        {
            Global::global_vm_env->Allocate(sizeof(jboolean), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jboolean*)*ret) = jni->CallBooleanMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        } 
        else if (!strcmp(_ret_type, "B") || !strcmp(_ret_type, "z"))
        {
            Global::global_vm_env->Allocate(sizeof(jbyte), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jbyte*)*ret) = jni->CallByteMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "C") || !strcmp(_ret_type, "c"))
        {
            Global::global_vm_env->Allocate(sizeof(jchar), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jchar*)*ret) = jni->CallCharMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "L") || !strcmp(_ret_type, "l"))
        {
            Global::global_vm_env->Allocate(sizeof(jlong), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jlong*)*ret) = jni->CallLongMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "F") || !strcmp(_ret_type, "f"))
        {
            Global::global_vm_env->Allocate(sizeof(jfloat), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jfloat*)*ret) = jni->CallFloatMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "D") || !strcmp(_ret_type, "d"))
        {
            Global::global_vm_env->Allocate(sizeof(jdouble), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jdouble*)*ret) = jni->CallDoubleMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "S") || !strcmp(_ret_type, "s"))
        {
            Global::global_vm_env->Allocate(sizeof(jshort), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jshort*)*ret) = jni->CallShortMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "I") || !strcmp(_ret_type, "i"))
        {
            Global::global_vm_env->Allocate(sizeof(jint), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jint*)*ret) = jni->CallIntMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        else if (!strcmp(_ret_type, "V") || !strcmp(_ret_type, "v"))
        {
            jni->CallObjectMethod(obj, mid);
        }
        else 
        {
            Global::global_vm_env->Allocate(sizeof(jobject), reinterpret_cast<Global::memory_alloc_ptr>(ret));
            *((jobject*)*ret) = jni->CallObjectMethodA(obj, mid, reinterpret_cast<jvalue*>(*values));
        }
        
        StringTool::DellocateChString(_desc);
        _desc = NULL;
        StringTool::DellocateChString(_class);
        _class = NULL;
        StringTool::DellocateChString(_method_name);
        _method_name = NULL;
        StringTool::DellocateChString(_method_type);
        _method_type = NULL;
        StringTool::DellocateChString(_ret_type);
        _ret_type = NULL;
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