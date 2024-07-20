#include "config.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace enum_ext {
template <>
auto to_string(const reb::config::ConfigValue value) noexcept
    -> std::string {
    return reb::config::ConfigValueMap[std::to_underlying(value)];
}

template <>
auto from_string(const std::string value) noexcept
    -> std::optional<reb::config::ConfigValue> {
    for (size_t i = 0;
         i < std::to_underlying(reb::config::ConfigValue::__count__); ++i)
        if (value == reb::config::ConfigValueMap[i])
            return (reb::config::ConfigValue)i;

    return {};
}
}  // namespace enum_ext
