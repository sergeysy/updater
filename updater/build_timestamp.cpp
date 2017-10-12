
#include "build_timestamp.h"
#include "build_defs.h"

namespace application {

std::string get_build_timestamp()
{
    static const char build_timestamp[] =
    {
        BUILD_YEAR_CH0,
        BUILD_YEAR_CH1,
        BUILD_YEAR_CH2,
        BUILD_YEAR_CH3,
        BUILD_MONTH_CH0,
        BUILD_MONTH_CH1,
        BUILD_DAY_CH0,
        BUILD_DAY_CH1,
        BUILD_HOUR_CH0,
        BUILD_HOUR_CH1,
        BUILD_MIN_CH0,
        BUILD_MIN_CH1,
        BUILD_SEC_CH0,
        BUILD_SEC_CH1
    };

    return std::string(build_timestamp, build_timestamp + sizeof(build_timestamp));
}

} // end namespace application
