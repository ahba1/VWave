#include <jvmti.h>
#include <string>

class VMError {

private:
    jvmtiError *error;

public:
    VMError(jvmtiError *error) {
        this->error = error;
    }

    std::string GetReason();    

    int GetErrorCode();
};