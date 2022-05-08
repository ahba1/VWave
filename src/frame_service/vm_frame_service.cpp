#include "../service_header/vm_frame_service.hpp"

namespace VMFrameService
{


    void _AllocateCapabilities()
    {
        jvmtiCapabilities caps;
        Logger::i("VMFrameService", "AllocateCapabilities");
        memset(&caps, 0, sizeof(caps));
        jvmtiError e = Global::global_vm_env->AddCapabilities(&caps);
        Exception::HandleException(e);
    }

    void Init(char **options, int option_size)
    {
        _AllocateCapabilities();
    }

    void GetCurrentMethodFrame(jthread thread, VMFrameHandler handler)
    {
        jvmtiError error;
        jvmtiFrameInfo info[Global::_stack_trace_depth];
        VMModel::StackFrame **frames;
        jint size;
        error = Global::global_vm_env->GetStackTrace(thread, 0, Global::_stack_trace_depth, info, &size);
        Exception::HandleException(error);
        if (size < 0)
        {
            return;
        }
        error = Global::global_vm_env->Allocate(sizeof(VMModel::StackFrame*) * (size), reinterpret_cast<Global::memory_alloc_ptr>(&frames));
        Exception::HandleException(error);
        for (int i = 0; i < size; i++)
        {
            VMModel::MapStackFrame(&info[i], &frames[i]);
        }
        handler(frames, size);
        for (int i = 0; i < size; i++)
        {
            VMModel::DellocateStackFrame(frames[i]);
        }
        error = Global::global_vm_env->Deallocate(reinterpret_cast<Global::memory_delloc_ptr>(frames));
        Exception::HandleException(error);
    }
}
