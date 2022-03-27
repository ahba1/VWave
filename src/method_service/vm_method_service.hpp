#include "../base/vm_service.hpp"
#include "../base/vm_model.hpp"

#include "../global.hpp"

using namespace std;
using namespace Global;

namespace VMModel
{

    class Method;

    void MapJMethod(jvmtiEnv *env, jmethodID methodID, Method *method);

    /**
     * @brief
     * format: [access] [static or not] [final or not] [generic] synchronized/native [return-type] [name]([param-type...])
     */
    void PrintJMethod(Method *method);
}

namespace _VMMethodService
{
    typedef void (*VMMethodHandler)(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);
}

class VMMethodService : public VMService
{
private:
    int _is_started = 0;

    void DispatchCMD(char *key, char *value = NULL);

    void RegisterNormalEventHandler();

    void RegisterMethodTraceHandler();

public:
    VMMethodService(jvmtiEnv *vm_env);

    VMMethodService();

    void ParseOptions(char **options, int option_size) override;

    char *GetServiceName() override;

    void AddEntryFilter(char *filter,_VMMethodService::VMMethodHandler handler);

    void AddExitFilter(char *filter,_VMMethodService::VMMethodHandler handler);
    
    void GetMethodTrace(char *file);

    ~VMMethodService();
};
