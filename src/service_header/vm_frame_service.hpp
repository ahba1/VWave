#include "vm_model.hpp"
#include "../global.hpp"

using namespace std;
using namespace Global;

namespace VMFrameService
{
    void Init();

    void GetCurrentMethodFrame(jthread thread, VMModel::StackFrame *frame, jint *size);
}