#include <fastcommon/shared_func.h>
#include <regex>

#include "vm_method_invoke.hpp"
#include "vm_jni_helper.hpp"

namespace MethodInvoke
{
    map<string, vector<VMModel::MethodInvokeTask *>> invoke_tasks;

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
            Exception::HandleException(error);
        }
        for (int i = 0; i < entry_count; i++)
        {
            if (!strcmp(table[i].name, "this"))
            {
                jvmtiError error = vm_env->GetLocalObject(thread, 0, table[i].slot, &ret);
                Exception::HandleException(error);
                break;
            }
        }
        for (int i = 0; i < entry_count; i++)
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

    void SetInvokeMethod(char *method_name, VMModel::MethodInvokeTask *_task)
    {
        if (invoke_tasks.count(method_name) == 0)
        {
            invoke_tasks[method_name] = vector<VMModel::MethodInvokeTask *>();
        }
        invoke_tasks[method_name].push_back(_task);
    }

    void OnMethodInvoke(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jobject obj = GetThis(vm_env, jni, thread, method);
        if (!invoke_tasks.count(method->name))
        {
            vector<VMModel::MethodInvokeTask *> tasks = invoke_tasks[method->name];
            for (int i = 0; i < tasks.size(); i++)
            {
                jvalue ret;
                jvalue *params;
                int params_len;
            }
        }
    }

    void InvokeInstanceMethod(jvmtiEnv *vm_env,
                              JNIEnv *jni,
                              jthread thread,
                              jobject target,
                              char *method,
                              jvalue *params,
                              jvalue *ret)
    {
        //解析方法描述符获得相关信息
        // eg. MethodTest::endMethodPrint()V
        // FullClassName::MethodName(ParamsDescptor)Ret

        char *class_name;
        char *method_name;
        char *method_sign;
        char *ret_type;
        GetFullClassName(method, &class_name);
        GetMethodName(method, &method_name);
        GetMethodDesc(method, &method_sign);
        GetMethodRetType(method, &ret_type);

        //根据返回值类型调用方法

        StringTool::DellocateChString(class_name);
        StringTool::DellocateChString(method_name);
        StringTool::DellocateChString(method_sign);
        StringTool::DellocateChString(ret_type);
    }

    void GetFullClassName(char *desc, char **ret)
    {
        StringTool::RegexSearch(desc, ".*(?=::)", ret);
    }

    void GetMethodName(char *desc, char **ret)
    {
        StringTool::RegexSearch(desc, "(?<=::).*(?=\()", ret);
    }

    void GetMethodDesc(char *desc, char **ret)
    {
        StringTool::RegexSearch(desc, "\(*\).*", ret);
    }

    void GetMethodRetType(char *desc, char **ret)
    {
        StringTool::RegexSearch(desc, "(?<=\)).*", ret);
    }

    void ConvertToJValue(JNIEnv *jni,
                         jthread thread,
                         VMModel::Method *method,
                         VMModel::MethodInvokeTask *task,
                         jvalue **params,
                         int *size)
    {
        *size = task->params_len;
        for (int i=0;i<*size;i++)
        {
            char *type = task->param_type[i];
            void *ptr = task->params;
            int type_len = VMModel::GetTypeLength(type, (void*)ptr);
            if (StringTool::RegexHas(type, "_basic"))
            {
                
            }
        }
    }
}