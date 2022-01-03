#include <set>
#include <regex>

#include "../vm_service_manager.hpp"
#include "../base/vm_model.hpp"

class VMMethodService: public VMService {

private:
    set<regex> filters;

public:
    VMMethodService(jvmtiEnv *vm_env);

    string GetServiceName() {
        return "ThreadService";
    }

    void RegisterEventHandler();

    void AddFilter(char *filter);
};
