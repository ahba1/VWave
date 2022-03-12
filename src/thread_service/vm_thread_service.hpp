#include "../base/vm_service.hpp"
#include "../base/vm_model.hpp"

#include "../global.hpp"

using namespace std;
using namespace Global;

namespace _VMThreadService
{
    typedef void (*ThreadWatcher)(jvmtiEnv *vm_env, JNIEnv *jni, VMModel::VMThread *vm_thread);

    typedef void* (*VWaveThreadFunc)(void *args);

    void CreateJNIThread(jthread *thread, VWaveThreadFunc func, void *args);

    void OnThreadStateChange(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread);
}

class VMThreadService : public VMService
{

private:
    int thread_state;

    void DispatchCMD(char *key, char *value = NULL);

    void StartMonitorThread();

    void EndMoitorThread();

public:
    VMThreadService(jvmtiEnv *vm_env);

    ~VMThreadService();

    void ParseOptions(char **options, int option_size) override;

    char *GetServiceName() override;

    void GetCurrentThreadInfo();

    void MonitorThread(char *thread_name, _VMThreadService::ThreadWatcher watcher);
};
