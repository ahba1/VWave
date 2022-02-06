#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>

namespace Exception {
    extern void HandleException(jvmtiError error);
}
#endif
