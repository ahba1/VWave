#include <vector>

#include "../vm_service_manager.hpp"
#include "../base/vm_model.hpp"

class VMThreadService: public VMService {

private:
    vector<VMModel::JVMThread*> threads;
    jthread *_jthreads;

    void clearCachedThreads();

public:
    VMThreadService(jvmtiEnv *vm_env);

    string GetName() {
        return "ThreadService";
    }

    void OnDestroyed();

    void GetAllThread();

    void DumpThreadsInfo(VMModel::Format *format, std::streambuf *target = std::cout.rdbuf());

}; 