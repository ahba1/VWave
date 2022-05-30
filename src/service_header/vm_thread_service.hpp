#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "../global.hpp"
#include "../data/vm_thread.hpp"

class VMServiceDesc
{
};

class VMThreadResolver
{
public:
        VMServiceDesc *Resolve(char *file);
};

class VMThreadService
{
private:
        VMThreadResolver resolver;
        vector<VMThread *> threads;
        char *_target;

public:
        VMThreadService();

        ~VMThreadService();

        void Init(uint8_t phase);

        void Invoke(char *target);

        void OnPhaseChaged(uint8_t phase);

        void OnThreadStart(jvmtiEnv *jvmti_env,
                           JNIEnv *jni_env,
                           jthread thread);

        void OnThreadEnd(jvmtiEnv *jvmti_env,
                         JNIEnv *jni_env,
                         jthread thread);

        void OnExplicitChange(jvmtiEnv *jvmti_env,
                              JNIEnv *jni_env,
                              jthread invoker,
                              jthread invokee,
                              char *method);

        void OnImplicitChangeStart(jvmtiEnv *jvmti_env,
                                   JNIEnv *jni_env,
                                   jthread invoker,
                                   jobject invokee,
                                   char *method);

        void OnImplicitChangeEnd(jvmtiEnv *jvmti_env,
                                   JNIEnv *jni_env,
                                   jthread invoker,
                                   jobject invokee,
                                   char *method);
        VMThreadResolver &GetResolver();
};