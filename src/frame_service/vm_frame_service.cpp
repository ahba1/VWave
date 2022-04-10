#include "service_header/vm_frame_service.hpp"

namespace VMFrameService
{

    void Init()
    {
    }

    void GetCurrentMethodFrame(jthread thread, VMModel::StackFrame **frame, jint *size)
    {
        jvmtiError error;
        jvmtiFrameInfo *info;
        error = Global::global_vm_env->GetStackTrace(thread, 0, Global::_stack_trace_depth, info, size);
        Exception::HandleException(error);
        if (*size < 0)
        {
            return;
        }
        error = Global::global_vm_env->Allocate(sizeof(VMModel::StackFrame) * (*size), reinterpret_cast<Global::memory_alloc_ptr>(frame));
        Exception::HandleException(error);
        for (int i = 0; i < *size; i++)
        {
            VMModel::MapStackFrame(&info[i], &frame[i]);
        }
    }
}
