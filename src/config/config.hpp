#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace reb::config {
enum class ConfigValue : size_t {
    COMP,
    FLAGS,
    SOURCE,
    BUILD,
    AUTO_RUN,
    IGNORE,
    __count__
};

static const std::string ConfigValueMap[std::to_underlying(ConfigValue::__count__)] = {
    "COMP",
    "FLAGS",
    "SOURCE",
    "BUILD",
    "AUTO_RUN",
    "IGNORE"
};


auto ReadConfig() -> void;
auto GetIgnoreList() -> std::vector<fs::path>;
}
