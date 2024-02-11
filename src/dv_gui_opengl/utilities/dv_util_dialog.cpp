#include "dv_gui_opengl/utilities/dv_util_dialog.hpp"
#include "pfd/portable-file-dialogs.h"

using namespace dvsku;

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

std::vector<std::string> dv_util_dialog::open_file(const std::string& title, const std::string& path, 
    std::vector<dv_file_filter> filters) 
{
    std::vector<std::string> converted_filters = convert_filters(filters);
    return pfd::open_file(title, path, converted_filters).result();
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE

std::vector<std::string> dvsku::dv_util_dialog::convert_filters(std::vector<dv_file_filter> filters) {
    std::vector<std::string> converted;

    for (const auto& ff : filters) {
        converted.push_back(ff.name);
        converted.push_back(ff.extensions);
    }

    return converted;
}