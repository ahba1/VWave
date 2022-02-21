#include "../base/vm_service.hpp"
#include "../base/vm_model.hpp"

#include "../global.hpp"

using namespace std;
using namespace Global;

namespace VMModel
{
    class VMThread;

    void MapVMThread(jvmtiEnv *env, jthread thread, VMThread *vm_thread);

    void PrintVMThread(VMThread *vm_thread);

    void MapThreadState(int state, char **state_str);

    void MapLocalInfo(VMThread *vm_thread);

    // when there is a call of MapVMThread, there is must a call of DellocateThread
    void DellocateThread(jvmtiEnv *env, VMThread *vm_thread);
}

namespace _VMThreadService
{
    typedef void (*ThreadWatcher)(jvmtiEnv *vm_env, JNIEnv *jni, VMModel::VMThread *vm_thread);
}

class VMThreadService : public VMService
{

private:
    int thread_state;

    void DispatchCMD(char *key, char *value = NULL);

public:
    VMThreadService(jvmtiEnv *vm_env);

    void ParseOptions(char **options, int option_size) override;

    char *GetServiceName() override;

    void GetCurrentThreadInfo();

    void MonitorThread(char *thread_name, _VMThreadService::ThreadWatcher watcher);
};
