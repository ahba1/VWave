#include "vwave_core.hpp"

namespace Exception {
    void HandleException(jvmtiError error) {
        switch (error)
        {
        case JVMTI_ERROR_NONE:
            break;        
        default:
            std::cout<<error<<"\n";
            throw error;
        }
    }

    void HandleExternalException(int error) {
        switch (error)
        {
        case 0:
            break;        
        default:
            std::cout<<error<<"\n";
            throw error;
        }
    }
}