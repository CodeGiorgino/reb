#include "config.hpp"

#include <cstddef>
#include <filesystem>
#include <utility>

#include "../context/context.hpp"

namespace reb::config {
auto ReadConfig() -> void {}

auto GetIgnoreList() -> std::vector<fs::path> {
    auto ignoreList = reb::context::Context.Config[std::to_underlying(ConfigValue::IGNORE)];
    std::vector<fs::path> retval{fs::current_path() / ".reb"};
    if (ignoreList.empty()) return retval;

    size_t pos;
    while ((pos = ignoreList.find(' ')) != std::string::npos) {
        const auto part = ignoreList.substr(0, pos);
        ignoreList.erase(0, pos);
        retval.push_back(fs::current_path() / part);
    };
    return retval;
}
}  // namespace reb::config

namespace enum_ext {
    template<typename>
    constexpr auto to_string(const reb::config::ConfigValue value) -> std::string {
        return reb::config::ConfigValueMap[std::to_underlying(value)]; 
    }

    template<typename>
    constexpr auto from_string(const std::string value) -> std::optional<reb::config::ConfigValue> {
        for (size_t i = 0; i < std::to_underlying(reb::config::ConfigValue::__count__); ++i)
            if (value == reb::config::ConfigValueMap[i]) return (reb::config::ConfigValue)i;

        return {};
    }
}
