#pragma once

#include "dv_gui_opengl/models/dv_command.hpp"
#include "dv_gui_opengl/utilities/dv_util_type_traits.hpp"

#include <map>

namespace dvsku {
    template<typename T>
    requires is_enum<T>
    class dv_sys_command {
    public:
        dv_sys_command()                      = default;
        dv_sys_command(const dv_sys_command&) = delete;
        dv_sys_command(dv_sys_command&&)      = delete;

        dv_sys_command& operator=(const dv_sys_command&) = delete;
        dv_sys_command& operator=(dv_sys_command&&)      = delete;

    public:
        // Set a command to execute
        void set_to_execute(T command_type) {
            if (!m_commands.contains(command_type))
                return;

            m_commands[command_type].to_execute = true;
        }

        // Check if command is set to execute
        bool& is_set_to_execute(T command_type) {
            if (!m_commands.contains(command_type))
                return false;

            return m_commands[command_type].to_execute;
        }

        // Execute all commands that are set to execute
        void execute_all() {
            for (auto& [type, command] : m_commands)
                command.execute();
        }

    protected:
        // Set command
        void set_command(T command_type, const dv_command& cmd) {
            if (m_commands.contains(command_type))
                return;

            m_commands.insert({ command_type, cmd });
        }

    private:
        std::map<T, dv_command> m_commands;
    };
}