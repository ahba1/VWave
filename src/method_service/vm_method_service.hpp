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

typedef void (*VMMethodHandler)(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);

class VMMethodService : public VMService
{
private:
    int _is_started = 0;

    void DispatchCMD(char *key, char *value=NULL);

    void RegisterEventHandler();


public:
    VMMethodService(jvmtiEnv *vm_env);

    void ParseOptions(char **options, int options_size) override;

    char *GetServiceName() override;

    void AddFilter(char *filter, VMMethodHandler handler);

    void GetMethodTrace(char *methodName);
};
