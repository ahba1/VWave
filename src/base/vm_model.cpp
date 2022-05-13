#include <iostream>
#include <fstream>

#include "../service_header/vm_model.hpp"
#include "../global.hpp"

namespace VMModel
{
    void MapVMThread(jvmtiEnv *env, jthread thread, VMThread *vm_thread)
    {
        jvmtiError error;
        vm_thread->_thread = thread;
        error = env->GetThreadState(thread, &vm_thread->_thread_state);
        Exception::HandleException(error);
        MapThreadState(vm_thread->_thread_state, &vm_thread->thread_state);
        // get vm thread info, info includes its name, priority, is_damon, thread group, classloader
        MapLocalInfo(env, vm_thread);
    }

    void PrintVMThread(VMThread *vm_thread)
    {
        cout << vm_thread->thread_name << " state: " << vm_thread->thread_state << "\n";
    }

    void MapThreadState(int state, char **state_str)
    {
        switch (state)
        {
        case 0x0000:
            *state_str = "New";
            break;
        case 0x0001:
            *state_str = "Alive";
            break;
        case 0x0002:
            *state_str = "Terminated";
            break;
        case 0x0004:
            *state_str = "Runnable";
            break;
        case 0x0400:
            *state_str = "Waiting for a lock";
            break;
        case 0x0080:
            *state_str = "Waiting";
            break;
        case 0x0010:
            *state_str = "Waiting without a timeout";
            break;
        case 0x0020:
            *state_str = "Waiting with a timeout";
            break;
        case 0x0100:
            *state_str = "Object waiting";
            break;
        case 0x0200:
            *state_str = "Parked";
            break;
        case 0x100000:
            *state_str = "Suspended";
            break;
        case 0x200000:
            *state_str = "Interrupted";
            break;
        case 0x400000:
            *state_str = "Native thread running";
            break;
        default:
            *state_str = "Unknowns";
        }
    }

    void MapLocalInfo(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Allocate(sizeof(jvmtiThreadInfo), reinterpret_cast<unsigned char **>(&vm_thread->_info));
        Exception::HandleException(error);
        error = env->GetThreadInfo(vm_thread->_thread, vm_thread->_info);
        Exception::HandleException(error);
        vm_thread->thread_name = vm_thread->_info->name;
        vm_thread->thread_priority = vm_thread->_info->priority;
        vm_thread->is_daemon = vm_thread->_info->is_daemon;
    }

    void DellocateThread(jvmtiEnv *env, VMThread *vm_thread)
    {
        jvmtiError error;
        error = env->Deallocate(reinterpret_cast<unsigned char *>(vm_thread->_info));
    }

    void TestClassLoad(jclass clazz) 
    {
        char *name;
        cout<<"?"<<endl;
        jvmtiError error = Global::global_vm_env->GetSourceFileName(clazz, &name);
        cout<<"??"<<endl;
        Exception::HandleException(error);
        Logger::d("TestClassLoad", name);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(name));
        Exception::HandleException(error);
    }
    
    void MapJClazz(jclass klazz, VMClazz **clazz)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(VMClazz), reinterpret_cast<Global::memory_alloc_ptr>(clazz));
        Exception::HandleException(error);
        VMClazz *_clazz = *clazz;
        error = Global::global_vm_env->Allocate(sizeof(ClazzMeta), reinterpret_cast<Global::memory_alloc_ptr>(&_clazz->meta));
        Exception::HandleException(error);
        _clazz->meta->_clazz = klazz;
        error = Global::global_vm_env->GetSourceFileName(klazz, &_clazz->source_file);
        Exception::HandleException(error);
        JNIEnv *jni;
        int ret = Global::AllocateJNIEnv(&jni);
        Exception::HandleExternalException(ret);
        jclass clazz_obj = jni->GetObjectClass(klazz);
        //sig rule: (params)return
        jmethodID mid = jni->GetMethodID(clazz_obj, "getName", "()Ljava/lang/String;");
        jstring jName = (jstring)jni->CallObjectMethod(klazz, mid);
        StringTool::ConvertJString(jName, &_clazz->full_name);
        jni->DeleteLocalRef(clazz_obj);
        jni->DeleteLocalRef(jName);
        Global::DeallocateJNIEnv(jni);
    }

    void DellcateClazz(VMClazz *clazz)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz->source_file));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(clazz));
        Exception::HandleException(error);
    }

    void GetMethodFullName(char **dest, Method *method, VMClazz *clazz)
    {
        StringTool::Concat(dest, {clazz->full_name, "::", method->name});
    }

    void MapJMethod(jmethodID methodID, Method **method)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(Method), reinterpret_cast<Global::memory_alloc_ptr>(method));
        Exception::HandleException(error);
        Method *_method = *method;
        error = Global::global_vm_env->Allocate(sizeof(Meta), reinterpret_cast<Global::memory_alloc_ptr>(&_method->meta));
        Exception::HandleException(error);
        _method->meta->_id = methodID;
        error = Global::global_vm_env->GetMethodDeclaringClass(methodID, &_method->meta->_clazz);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetMethodName(methodID, &_method->name, &_method->signature, &_method->generic);
        Exception::HandleException(error);
        error = Global::global_vm_env->GetMethodModifiers(methodID, &_method->access_flag);
        Exception::HandleException(error);
        error = Global::global_vm_env->IsMethodNative(methodID, &_method->is_native);
        Exception::HandleException(error);
    }

    void DellocateMethod(Method *method)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->signature));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method->generic));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(method));
        Exception::HandleException(error);
    }

    void CreateMethodFrame(MethodFrame **mf, const char *method)
    {
        using namespace std::chrono;
        jvmtiError error;
        time_point<high_resolution_clock> _time = high_resolution_clock::now();
        error = Global::global_vm_env->Allocate(sizeof(MethodFrame), reinterpret_cast<Global::memory_alloc_ptr>(mf));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(strlen(method) + 1, reinterpret_cast<Global::memory_alloc_ptr>(&(*mf)->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(sizeof(time_point<high_resolution_clock>), reinterpret_cast<Global::memory_alloc_ptr>(&(*mf)->tm));
        Exception::HandleException(error);
        strcpy((*mf)->name, method);
        *((*mf)->tm) = _time;
    }

    void DellocateMethodFrame(MethodFrame *mf)
    {
        jvmtiError error;
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf->tm));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf->name));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(mf));
        Exception::HandleException(error);
    }

    void MapStackFrame(jvmtiFrameInfo *info, StackFrame **sf)
    {
        jvmtiError error;
        error = Global::global_vm_env->Allocate(sizeof(StackFrame), reinterpret_cast<Global::memory_alloc_ptr>(sf));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(sizeof(StackFrameMeta), reinterpret_cast<Global::memory_alloc_ptr>(&(*sf)->meta));
        Exception::HandleException(error);
        MapJMethod(info->method, &(*sf)->vm_method);
        Logger::d("Map", (*sf)->vm_method->name);
    }

    void DellocateStackFrame(StackFrame *sf)
    {
        jvmtiError error;
        DellocateMethod(sf->vm_method);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(sf->meta));
        Exception::HandleException(error);
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(sf));
        Exception::HandleException(error);
    }

    void ConvertFilter(VMModel::MethodTask *task, Json::Value &filter)
    {
        int count = filter.size();
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(char*) * count, reinterpret_cast<Global::memory_alloc_ptr>(&task->method_filter));
        Exception::HandleException(error);
        for (int i=0;i<count;i++)
        {
            StringTool::Copy(&task->method_filter[i], filter[i].asCString());
        }
        task->filter_len = count;
    }

    int GetTypeLength(const char *type)
    {
        if (!strcmp(type, "boolean"))
        {
            return 1;
        }
        if (!strcmp(type, "char"))
        {
            return 2;
        }
        if (!strcmp(type, "short"))
        {
            return 2;
        }
        if (!strcmp(type, "int"))
        {
            return 4;
        }
        if (!strcmp(type, "long"))
        {
            return 8;
        }
        if (!strcmp(type, "float"))
        {
            return 4;
        }
        if (!strcmp(type, "double"))
        {
            return 8;
        }
        if (!strcmp(type, "void"))
        {
            return 0;
        }
        return sizeof(jobject);
    }

    int GetParamsLength(Json::Value &param_types)
    {
        int count = param_types.size();
        int len = 0;
        for (int i=0;i<count;i++)
        {
            len += GetTypeLength(param_types[i].asCString());
        }
        return len;
    }

    void InitParams(const char *type, Json::Value value, char *ptr)
    {
        if (!strcmp(type, "boolean"))
        {
            *((bool*)ptr) = value.asBool();
            return;
        }
        if (!strcmp(type, "char"))
        {
            *ptr = *value.asCString();
            return;
        }
        if (!strcmp(type, "short"))
        {
            *((int16_t*)ptr) = value.asInt();
        }
        if (!strcmp(type, "int"))
        {
            *((int32_t*)ptr) = value.asInt();
        }
        if (!strcmp(type, "long"))
        {
            *((int64_t*)ptr) = value.asInt64();
        }
        if (!strcmp(type, "float"))
        {
            *((_Float32*)ptr) = value.asFloat();
        }
        if (!strcmp(type, "double"))
        {
            *((_Float64*)ptr) = value.asDouble();
        }
    }

    void ConvertMethodInvokeTask(VMModel::MethodInvokeTask *task, Json::Value &method_invoke_task)
    {
        StringTool::Copy(&task->filter, method_invoke_task["filter"].asCString());
        StringTool::Copy(&task->name, method_invoke_task["name"].asCString());
        StringTool::Copy(&task->result_type, method_invoke_task["result_type"].asCString());
        task->params_len = method_invoke_task["params_len"].asInt();

        Json::Value params = method_invoke_task["params"];
        Json::Value param_types = method_invoke_task["param_types"];
        jvmtiError error = Global::global_vm_env->Allocate(GetParamsLength(param_types), reinterpret_cast<Global::memory_alloc_ptr>(&task->params));
        Exception::HandleException(error);
        error = Global::global_vm_env->Allocate(param_types.size() * sizeof(char*), reinterpret_cast<Global::memory_alloc_ptr>(&task->param_type));
        Exception::HandleException(error);
        char *ptr=(char*)task->params;
        for (int i=0;i<param_types.size();i++)
        {
            InitParams(param_types[i].asCString(), params[i], ptr);
            ptr += GetTypeLength(param_types[i].asCString());
            StringTool::Copy(&task->param_type[i], param_types[i].asCString());
        }
    }

    void ConvertParamReadTask(VMModel::ParamReadTask *task, Json::Value &param_read_task)
    {
        StringTool::Copy(&task->filter, param_read_task["filter"].asCString());
        int count = param_read_task["names"].size();
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(char*) * count, reinterpret_cast<Global::memory_alloc_ptr>(&task->names));
        Exception::HandleException(error);
        for (int i=0;i<count;i++)
        {
            StringTool::Copy(&task->names[i], param_read_task[i].asCString());
        }
        task->names_len = count;
    }

    VMModel::MethodTask *ConvertToMethodTask(char *file)
    {
        Json::Value root;
        Json::Reader reader;
        std::fstream task_file_ifs;
        task_file_ifs.open(file);
        if (!task_file_ifs.is_open())
        {
            Logger::e("MethodTask", "read json file failed");
            return NULL;
        }
        if (!reader.parse(task_file_ifs, root, true))
        {
            Logger::e("MethodTask", "parse failed");
            return NULL;
        }
        Json::Value filter = root["method_filter"];
        Json::Value method_invoke_task = root["method_invoke_tasks"];
        Json::Value param_read_task = root["param_read_tasks"];
        VMModel::MethodTask *ret;
        if (!filter.isArray())
        {
            Logger::e(file, "filter is not an array");
            return NULL;
        }
        if (!method_invoke_task.isArray())
        {
            Logger::e(file, "method_invoke_task is not an array");
            return NULL;
        }
        if (!param_read_task.isArray())
        {
            Logger::e(file, "param_read_task is not an array");
            return NULL;
        }
        jvmtiError error = Global::global_vm_env->Allocate(sizeof(VMModel::MethodTask), reinterpret_cast<Global::memory_alloc_ptr>(&ret));
        Exception::HandleException(error);
        ConvertFilter(ret, filter);
        error = Global::global_vm_env->Allocate(sizeof(VMModel::MethodInvokeTask) * method_invoke_task.size(), reinterpret_cast<Global::memory_alloc_ptr>(&ret->method_invoke_tasks));
        Exception::HandleException(error);
        for (int i=0;i<method_invoke_task.size();i++)
        {
            ConvertMethodInvokeTask(&ret->method_invoke_tasks[i], method_invoke_task[i]);
        }
        error = Global::global_vm_env->Allocate(sizeof(VMModel::ParamReadTask) * method_invoke_task.size(), reinterpret_cast<Global::memory_alloc_ptr>(&ret->param_read_tasks));
        Exception::HandleException(error);
        for (int i=0;i<param_read_task.size();i++)
        {
            ConvertParamReadTask(&ret->param_read_tasks[i], param_read_task[i]);
        }
        task_file_ifs.close();
        Logger::t("TestConvertToMethodTask", "Successfully");
        return ret;
    }   

    void DeallocateMethodInvokeTask(VMModel::MethodInvokeTask *task)
    {
        StringTool::DellocateChString(task->filter);
        StringTool::DellocateChString(task->name);
        StringTool::DellocateChString(task->result_type);
        jvmtiError e = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(task->params));
        Exception::HandleException(e);
        for (int i=0;i<task->params_len;i++)
        {
            StringTool::DellocateChString(task->param_type[i]);
        }
        e = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(task->param_type));
        Exception::HandleException(e);
        e = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(task));
        Exception::HandleException(e);
    }

    void DeallocateParamReadTask(VMModel::ParamReadTask *task)
    {
        StringTool::DellocateChString(task->filter);
        for (int i=0;i<task->names_len;i++)
        {
            StringTool::DellocateChString(task->names[i]);
        }
        jvmtiError e = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(task->names));
        Exception::HandleException(e);
        e = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(task));
        Exception::HandleException(e);
    }

    void DeallocateMethodTask(VMModel::MethodTask *task)
    {
        if (task == NULL)
        {
            return;
        }
        for (int i=0;i<task->filter_len;i++)
        {
            StringTool::DellocateChString(task->method_filter[i]);
        }
        DeallocateMethodInvokeTask(task->method_invoke_tasks);
        DeallocateParamReadTask(task->param_read_tasks);
    }

    void PrintMethodTask(VMModel::MethodTask *task)
    {
        for (int i=0;i<task->filter_len;i++)
        {
            Logger::d("MethodTask::filter", task->method_filter[i]);
        }
    }

    void TestConvertToMethodTask()
    {
        VMModel::MethodTask *task = ConvertToMethodTask("./target.json");
        Logger::Assert("TestConvertToMethodTask", "Successfully");
    }
}