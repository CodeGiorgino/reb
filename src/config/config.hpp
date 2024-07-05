#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace reb::config {
enum ConfigValue : size_t {
    COMP,
    FLAGS,
    SOURCE,
    BUILD,
    AUTO_RUN,
    IGNORE,
    __count__
};

static const std::string ConfigValueMap[ConfigValue::__count__] = {
    "COMP",
    "FLAGS",
    "SOURCE",
    "BUILD",
    "AUTO_RUN",
    "IGNORE"
};


auto ReadConfig() -> void;
auto GetIgnoreList() -> std::vector<std::string>;
}

namespace enum_ext {
    constexpr auto to_string(reb::config::ConfigValue value) -> std::string {
        return reb::config::ConfigValueMap[value]; 
    }

    template<typename>
    constexpr auto from_string(std::string value) -> std::optional<reb::config::ConfigValue> {
        for (size_t i = 0; i < reb::config::ConfigValue::__count__; ++i)
            if (value == reb::config::ConfigValueMap[i]) return (reb::config::ConfigValue)i;

        return {};
    }
}
