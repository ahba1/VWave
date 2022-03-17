#ifndef VWAVE_CORE_H
#define VWAVE_CORE_H

#include <iostream>
#include <jvmti.h>

namespace Exception {
    extern void HandleException(jvmtiError error);

    extern void HandleException(int error);

    extern void HandleException(jint error);
}

namespace FileTool {
    extern int Start();

    extern int Stop();

    extern int Output(char *path, char *content, int len);
}
#endif
