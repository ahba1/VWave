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
}