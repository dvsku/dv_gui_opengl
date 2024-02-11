#include "dv_gui_opengl/utilities/dv_util_dialog.hpp"
#include "pfd/portable-file-dialogs.h"

using namespace dvsku;

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

std::vector<std::string> dv_util_dialog::open_file(const std::string& title, const std::string& path, 
    std::vector<std::string> filters)
{
    return pfd::open_file(title, path, filters).result();
}
