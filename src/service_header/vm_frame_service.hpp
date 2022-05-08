#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm_model.hpp"
#include "../global.hpp"

using namespace std;
using namespace Global;

namespace VMFrameService
{
    typedef void (*VMFrameHandler)(VMModel::StackFrame **frame, jint size);

    void Init(char **options, int option_size);

    void GetCurrentMethodFrame(jthread thread, VMFrameHandler handler);
}

#endif