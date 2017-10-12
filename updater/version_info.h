#pragma once


namespace application {

class version_info
{
public:

    static uint32_t major();
    static uint32_t middle();
    static uint32_t minor();
    static uint32_t build();

    static std::string to_string();

private:

    static const uint32_t info[4];

};

} // end namespace application
