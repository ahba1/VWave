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

namespace VMMethodService
{
    typedef void (*VMMethodHandler)(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);

    void Init(char **options, int option_size);

    void DefaultVMMethodHandler(jvmtiEnv *vm_env, JNIEnv *jni, jthread thread, VMModel::Method *method);

    void AddEntryFilter(char *filter, VMMethodHandler handler);

    void AddExitFilter(char *filter, VMMethodHandler handler);

    void RecordMethod(char *file);

    void Release();
}
