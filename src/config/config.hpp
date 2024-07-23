#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "../lib/utils.hpp"

namespace reb::config {
enum class ConfigValue : size_t {
    EXT,
    NAME,
    COMP,
    FLAGS,
    SOURCE,
    BUILD,
    AUTO_RUN,
    IGNORE,
    __count__
};

static const std::string
    ConfigValueMap[std::to_underlying(ConfigValue::__count__)] = {
        "EXT", "NAME", "COMP", "FLAGS", "SOURCE", "BUILD", "AUTO_RUN", ""};
}  // namespace reb::config

namespace enum_ext {
template <>
auto to_string(const reb::config::ConfigValue value) noexcept
    -> std::string;

template <>
auto from_string<reb::config::ConfigValue>(
    const std::string value) noexcept
    -> std::optional<reb::config::ConfigValue>;
}  // namespace enum_ext
