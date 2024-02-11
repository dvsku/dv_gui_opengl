#pragma once

#include <string>
#include <vector>

namespace dvsku {
    class dv_util_dialog {
    public:
        static std::vector<std::string> open_file(const std::string& title, const std::string& path, 
            std::vector<std::string> filters = { "All Files", "*" });
    };
}