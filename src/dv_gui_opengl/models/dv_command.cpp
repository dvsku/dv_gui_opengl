#include "dv_gui_opengl/models/dv_command.hpp"

using namespace dvsku;

dv_command::dv_command(command_fn_t fn) {
    m_fn = fn;
}

void dv_command::execute() {
    if (!to_execute || !m_fn) return;

    if (m_fn() == dv_command_state::finished)
        to_execute = false;
}
