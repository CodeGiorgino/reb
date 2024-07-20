#pragma once
#include <array>
#include <filesystem>
#include <string>
#include <utility>

#include "../config/config.hpp"
namespace fs = std::filesystem;

namespace reb::context {
struct Context final{
    std::string ProgramName{};
    std::string Command{};
    std::string Params{};
    fs::path ConfigPath{fs::path(std::getenv("HOME")) / ".config/reb"};
    std::array<std::string, std::to_underlying(reb::config::ConfigValue::__count__)> Config{""};
};
}  // namespace reb::context
