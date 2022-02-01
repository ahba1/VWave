#include <map>
#include <regex>

#include "../vm_service_manager.hpp"
#include "../base/include/vm_model.hpp"

class VMMethodService: public VMService {

private:
    map<regex, VMMethodHandler> filters;

public:
    VMMethodService(jvmtiEnv *vm_env);

    string GetServiceName() {
        return "ThreadService";
    }

    void RegisterEventHandler();

    void AddFilter(char *filter, VMMethodHandler handler);

    void GetMethodTrace(char *methodName);
};

typedef void(*VMMethodHandler)(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID);


void DefVMMethodHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID) {}
