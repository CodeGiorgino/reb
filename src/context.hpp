#pragma once
#include <filesystem>
#include <string>

#include "../deps/include/cppjson.hpp"
namespace fs = std::filesystem;

namespace reb::context {
struct Context final {
    std::string ProgramName{};
    std::string Command{};
    std::string Params{};
    fs::path ConfigPath{fs::path(std::getenv("HOME")) / ".config/reb"};
    json::json_node Config{};
};
}  // namespace reb::context
