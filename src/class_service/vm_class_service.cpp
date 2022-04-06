#include "vm_class_service.hpp"

namespace VMClassService
{
    void _AllocateCapabilities()
    {
        jvmtiCapabilities caps;
        Logger::i("VMClassService", "AllocateCapabilities");
        memset(&caps, 0, sizeof(caps));
        caps.can_get_source_file_name = 1;
        jvmtiError e = Global::global_vm_env->AddCapabilities(&caps);
        Exception::HandleException(e);
    }

    void Init(char **options, int option_size)
    {
        _AllocateCapabilities();
    }
}