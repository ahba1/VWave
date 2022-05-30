#include "../service_header/vwave_core.hpp"
#include "../global.hpp"

class VMData
{
public:
    virtual void Map(jobject meta) = 0;
    virtual void Println(std::ostream &os) = 0;
};