#pragma once

#include <string>
#include <vector>

namespace dvsku {
    class dv_util_dialog {
    public:
        static std::vector<std::string> open_file(const std::string& title, const std::string& path, 
            std::vector<std::string> filters = { "All Files", "*" });

        static std::string save_file(const std::string& title, const std::string& path,
            bool confirm_override = true, std::vector<std::string> filters = { "All Files", "*" });

        static std::string select_dir(const std::string& title, const std::string& path);

        static void message_error(const std::string& title, const std::string& text);
    };
}