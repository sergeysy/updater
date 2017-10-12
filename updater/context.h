#pragma once

#include <string>

namespace application {

struct context
{
    std::string cardreader;
    std::string barcode;
    std::string pidfile;
    std::string gprs_device;

    context& operator=(const context& src);

    bool process_options(int argc, char** argv, std::string& err);
};

} // end namespace application
