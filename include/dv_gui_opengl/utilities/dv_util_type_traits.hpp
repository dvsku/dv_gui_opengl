#pragma once

#include <type_traits>

namespace dvsku {
    template<typename T>
    concept is_enum = std::is_enum_v<T>;
}