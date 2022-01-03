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

namespace VMException {
    
    typedef int ERROR_TYPE; 

    void CheckException(jvmtiError error) {
        if (error != JVMTI_ERROR_NONE) {
            throw VMError(&error);
        }
    }
}

namespace JVM_INTERNAL_EXCEPTION {
    VMException::ERROR_TYPE ERROR_INTERNAL = 113;
    VMException::ERROR_TYPE NULL_POINTER = 100;
    VMException::ERROR_TYPE OUT_OF_MEMORY = 110;
}

namespace JVM_EXTERNAL_EXCEPTION {
    VMException::ERROR_TYPE UNSUPORTED_FUNCTION = 111;
    VMException::ERROR_TYPE UNATTACHED_THREAD = 115;
    VMException::ERROR_TYPE INVALID_ENVIRONMENT = 116;
    VMException::ERROR_TYPE WRONG_PHASE = 112;
}