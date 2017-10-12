#include <string>
#include <sstream>

#include "version_info.h"
#include "product_version.h"

namespace application {

const uint32_t version_info::info[4] = { version::MAJOR, version::MIDDLE, version::MINOR, version::BUILD };

uint32_t version_info::major()	{ return info[0]; }
uint32_t version_info::middle()	{ return info[1]; }
uint32_t version_info::minor()	{ return info[2]; }
uint32_t version_info::build()	{ return info[3]; }

std::string version_info::to_string()
{
    std::ostringstream oss;

    oss << (int)major()  << '.';
    oss << (int)middle() << '.';
    oss << (int)minor()  << '.';
    oss << (int)build();

    return oss.str();
}

} // end namespace application
