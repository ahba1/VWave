#pragma once
#ifndef _INCLUDE_GLOBAL_H
#define _INCLUDE_GLOBAL_H

#include <iostream>

#include <jvmti.h>

using namespace std;

namespace Global {
    typedef unsigned char** memory_alloc_ptr;
    typedef unsigned char* memory_delloc_ptr;

    inline JavaVM *global_java_vm;
    inline jvmtiEnv *global_vm_env;

    inline streambuf *globalOuter = cout.rdbuf();

    inline const char _spilt_token = '-';
    inline const char _spilt_kv_token = '=';
    inline const char _max_options_size = 10;
    inline const char _max_kv_size = 2;
    inline const char _stack_trace_depth = 10;

    inline const char* _param_1 = "service";

    inline const char* support_service[] = {"method"};
    inline const int support_service_size = 1;

    inline const int ERROR_JAVA_VM_NULL = -1;
    inline const int ERROR_NONE = 0;

    int AllocateJNIEnv(JNIEnv **env);

    int DeallocateJNIEnv(JNIEnv *env);
}

#endif