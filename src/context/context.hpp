#pragma once
#include <array>
#include <filesystem>
#include <string>

#include "../config/config.hpp"
namespace fs = std::filesystem;

namespace reb::context {
static struct {
    std::string ProgramName{};
    std::string Command{};
    std::string Params{};
    fs::path ConfigPath{fs::path(std::getenv("HOME")) / ".config/reb"};
    std::array<std::string, reb::config::ConfigValue::__count__> Config{""};
} Context;
}  // namespace reb::context
