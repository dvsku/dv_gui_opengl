#pragma once

#include "dv_gui_opengl/models/dv_file_filter.hpp"

#include <string>
#include <vector>

namespace dvsku {
    class dv_util_dialog {
    public:
        static std::vector<std::string> open_file(const std::string& title, const std::string& path, 
            std::vector<dv_file_filter> filters = { { "All Files", "*" } });

    private:
        static std::vector<std::string> convert_filters(std::vector<dv_file_filter> filters);
    };
}