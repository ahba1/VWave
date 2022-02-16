#pragma once
#ifndef _INCLUDE_GLOBAL_H
#define _INCLUDE_GLOBAL_H

#include <map>
#include <iostream>

#include <jvmti.h>

#include "base/vwave_core.hpp"
#include "base/vm_service.hpp"

using namespace std;

namespace Global {
    inline jvmtiEnv *global_vm_env;

    inline map<char*, VMService*> services;
    inline streambuf *globalOuter = cout.rdbuf();

    inline const char _spilt_token = '/';
    inline const char _spilt_kv_token = '=';
    inline const char _max_options_size = 10;
    inline const char _max_kv_size = 2;

    inline const char* _param_1 = "service";

    inline const char* support_service[] = {"method"};
    inline const int support_service_size = 1;
}

#endif