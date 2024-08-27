#pragma once
#include <filesystem>
#include <string>

#include "../deps/include/cppjson.hpp"
namespace fs = std::filesystem;

namespace reb::enviroment {
struct enviroment final {
    std::string programName{};
    std::string command{};
    std::string params{};
    fs::path configPath{fs::path(std::getenv("HOME")) / ".config/reb"};
    json::json_node configJson{};
};
}  // namespace reb::enviroment
