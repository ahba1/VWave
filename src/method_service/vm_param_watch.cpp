#include <map>
#include <vector>
#include <algorithm>
#include <sstream>

#include "vm_param_watch.hpp"

namespace ParamWatch
{
    map<string, vector<string>> watch_fields;
    map<string, map<string, string>> watch_fields_types;
    
    void GetIntValue(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot)
    {
        jint value = 0;
        jvmtiError error = vm_env->GetLocalInt(thread, 0, slot, &value);
        Exception::HandleException(error);
        std::ostringstream buffer;
        buffer << "Param " << name << " is " << value;
        Logger::i("FieldWatch", buffer.str().c_str());
    }

    void GetLongValue(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot)
    {
        jlong value = 0;
        jvmtiError error = vm_env->GetLocalLong(thread, 0, slot, &value);
        Exception::HandleException(error);
        std::ostringstream buffer;
        buffer << "Param " << name << " is " << value;
        Logger::i("FieldWatch", buffer.str().c_str());
    }

    void GetFloatValue(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot)
    {
        jfloat value = 0;
        jvmtiError error = vm_env->GetLocalFloat(thread, 0, slot, &value);
        Exception::HandleException(error);
        std::ostringstream buffer;
        buffer << "Param " << name << " is " << value;
        Logger::i("FieldWatch", buffer.str().c_str());
    }

    void GetDoubleValue(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot)
    {
        jdouble value = 0;
        jvmtiError error = vm_env->GetLocalDouble(thread, 0, slot, &value);
        Exception::HandleException(error);
        std::ostringstream buffer;
        buffer << "Param " << name << " is " << value;
        Logger::i("FieldWatch", buffer.str().c_str());
    }

    void GetObject(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot)
    {
        jobject value;
        jvmtiError error = vm_env->GetLocalObject(thread, 0, slot, &value);
        Exception::HandleException(error);
    }

    void GetValue(jvmtiEnv *vm_env, jthread thread, const char *name, jint slot, const char *type)
    {
        if (!strcmp(type, "boolean"))
        {
            GetIntValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "byte"))
        {
            GetIntValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "char"))
        {
            GetIntValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "short"))
        {
            GetIntValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "int"))
        {
            GetIntValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "long"))
        {
            GetLongValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "float"))
        {
            GetFloatValue(vm_env, thread, name, slot);
        }
        if (!strcmp(type, "double"))
        {
            GetDoubleValue(vm_env, thread, name, slot);
        }
    }

    void SetWatchField(char *method_name, char *param_name, char *type)
    {
        if (watch_fields.count(method_name) == 0)
        {
            watch_fields[method_name] = vector<string>();
            watch_fields_types[method_name] = map<string, string>();
        }
        watch_fields[method_name].push_back(param_name);
        watch_fields_types[method_name][param_name] = type;
    }
    
    void OnWatchField(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method)
    {
        jvmtiError error;
        jint entry_count;
        jvmtiLocalVariableEntry *table;
        error = vm_env->GetLocalVariableTable(method->meta->_id, &entry_count, &table);
        if (error == JVMTI_ERROR_ABSENT_INFORMATION)
        {
            Exception::HandleException(error, "recompile source file with -g");
        }
        else 
        {
            Exception::HandleException(error);
        }
        Logger::i("FieldWatch", method->name);
        for (int i=0;i<entry_count;i++)
        {
            if (watch_fields.count(method->name) != 0)
            {
                vector<string> &fields = watch_fields[method->name];

                if (find(fields.begin(), fields.end(), table[i].name) != fields.end())
                {
                    GetValue(vm_env, thread, table[i].name, table[i].slot, watch_fields_types[method->name][table[i].name].c_str());
                }
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
    }
}