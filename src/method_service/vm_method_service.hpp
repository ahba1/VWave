#include <map>
#include <regex>

#include "../base/vm_service.hpp"

using namespace std;

typedef void(*VMMethodHandler)(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, jmethodID methodID);

class VMMethodService: public VMService {

private:
    map<regex, VMMethodHandler> filters;

public:
    VMMethodService(jvmtiEnv *vm_env);

    char* GetServiceName() {
        return "MethodService";
    }

    void RegisterEventHandler();

    void AddFilter(char *filter, VMMethodHandler handler);

    void GetMethodTrace(char *methodName);
};

