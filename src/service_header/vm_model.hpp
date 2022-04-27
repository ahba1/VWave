#ifndef VM_MODEL_H
#define VM_MODEL_H

#include <iostream>
#include <jvmti.h>
#include <chrono>
#include "vwave_core.hpp"

namespace VMModel {
    class VMThread
    {
    public:
        jthread _thread;
        jint _thread_state;
        char *thread_state;
        jvmtiThreadInfo *_info;
        char *thread_name;
        jint thread_priority;
        jboolean is_daemon;
        // temprorily not add group info and class loader info
    };

    void MapVMThread(jvmtiEnv *env, jthread thread, VMThread *vm_thread);

    void PrintVMThread(VMThread *vm_thread);

    void MapThreadState(int state, char **state_str);

    void MapLocalInfo(jvmtiEnv *env, VMThread *vm_thread);

    // when there is a call of MapVMThread, there is must a call of DellocateThread
    void DellocateThread(jvmtiEnv *env, VMThread *vm_thread);
/***********************************Access Flag(method class)***************************************/

    typedef jint AccessFlag;

    const AccessFlag ACC_PUBLIC = 1;
    const AccessFlag ACC_PRIVATE = 1 << 1;
    const AccessFlag ACC_PROTECTED = 1 << 3;
    const AccessFlag ACC_FINAL = 1 << 4;
    const AccessFlag ACC_SYNCHRONIZED = 1 << 5;
    const AccessFlag ACC_BRIDGE = 1 << 6; //a bridge method that generated by compiler
    const AccessFlag ACC_VARARGS = 1 << 7; //declared with a variable number of argument
    const AccessFlag ACC_NATIVE = 1 << 8;
    const AccessFlag ACC_ABSTRACT = 1 << 9;
    const AccessFlag ACC_STRICT = 1 << 10;
    const AccessFlag ACC_SYNTHETIC = 1 << 11; //declare syntheic; not present in the source code

    bool ContainAccessFlag(AccessFlag source_flag, AccessFlag target);

/***********************************Thread State****************************************************/

    typedef jint ThreadState;

    const ThreadState NOT_ALIVE = 0;
    const ThreadState ALIVE = 1;
    const ThreadState TERMINATED = 1 << 1;
    const ThreadState RUNNABLE = 1 << 2;
    const ThreadState BLOCKED_ON_MONITOR_ENTER = 1 << 10;
    const ThreadState WAITING = 1 << 7;
    const ThreadState WAITING_INDEFINITELY = 1 << 4;
    const ThreadState WAITING_WITH_TIMEOUT = 1 << 5;
    const ThreadState SLEEPING = 1 << 6;
    const ThreadState IN_OBJECT_WAIT = 1 << 8;
    const ThreadState PARKED = 1 << 9;
    const ThreadState SUSPENDED = 1 << 20;
    const ThreadState INTERRUPTED = 1 << 21; 
    const ThreadState IN_NATIVE = 1 << 22;

    struct ClazzMeta
    {
        jclass _clazz;
    };

    struct VMClazz
    {
        ClazzMeta *meta;
        char *source_file;
        char *package_name;
    };

    extern void MapJClazz(jclass klazz, VMClazz **clazz);

    extern void DellcateClazz(VMClazz *clazz);

    struct Meta
    {
        jmethodID _id;
        jclass _clazz;
    };

    struct Method
    {
        Meta *meta;
        char *name;
        char *signature;
        char *generic;
        jint access_flag;
        jboolean is_native;
    };

    void MapJMethod(jmethodID methodID, Method **method);

    void DellocateMethod(Method *method);

    struct MethodFrame
    {
        char *name;
        std::chrono::time_point<std::chrono::high_resolution_clock> *tm;
    };

    void CreateMethodFrame(MethodFrame **mf, const char *method);

    void DellocateMethodFrame(MethodFrame *mf);

    struct StackFrameMeta
    {
        jvmtiFrameInfo *_frame_info;
        jvmtiStackInfo *_owner_info;
    };

    struct StackFrame
    {
        StackFrameMeta *meta;
        Method *vm_method;
    };

    void MapStackFrame(jvmtiFrameInfo *info, StackFrame **sf);

    void DellocateStackFrame(StackFrame *sf);
}
#endif