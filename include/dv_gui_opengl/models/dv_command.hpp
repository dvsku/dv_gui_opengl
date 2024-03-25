#pragma once

#include <functional>

namespace dvsku {
    enum class dv_command_state : unsigned char {
        finished = 0,
        repeat   = 1
    };

    class dv_command {
    public:
        using command_fn_t = std::function<dv_command_state()>;

    public:
        bool to_execute = false;

    public:
        dv_command() = default;
        dv_command(command_fn_t fn);

    public:
        void execute();

    private:
        command_fn_t m_fn = nullptr;
    };
}