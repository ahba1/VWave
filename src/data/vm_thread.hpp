#include "vm_data.hpp"

class VMThread: public VMData
{
public:
    char *name;
    int priority;
    bool is_daemon;
    uint8_t state;
    jthread meta;
public:
    void Map(jobject meta)
    {
        if (typeid(meta) != typeid(jthread))
        {
            //日志记录
            return;
        }
        jvmtiError error;
        jvmtiThreadInfo thread_info;
        jthread thr = (jthread)meta;
        this->meta = thr;
        error = Global::global_vm_env->GetThreadInfo(thr, &thread_info);
        if (JVMTI_ERROR_WRONG_PHASE == error || JVMTI_ERROR_INVALID_THREAD == error)
        {
            return;
        }
        Exception::HandleException(error);
        StringTool::Copy(&name, thread_info.name);
        priority = thread_info.priority;
        is_daemon = thread_info.is_daemon;
        state = 0;
    }

    void Println(std::ostream &os)
    {
        os << name << "-" << priority << "-" << is_daemon << "-" << state << std::endl;
    }
};