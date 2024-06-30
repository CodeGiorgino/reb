#pragma once
#include <string.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using std::string_literals::operator""s;
namespace fs = std::filesystem;

namespace reb {
#define REB_LINE_POS "\033[4m" << __FILE__ << ":" << __LINE__ << ":\033[0m \t"
#define REB_INFO(text) \
    { std::cout << REB_LINE_POS << "\033[33;1mINFO\033[0m\t" << text << "\n"; }
#define REB_ERROR(text) \
    { std::cout << REB_LINE_POS << "\033[31;1mERROR\033[0m\t" << text << "\n"; }
#define REB_PANIC(text)                                                          \
    {                                                                            \
        std::cout << REB_LINE_POS << "\033[31;1mERROR\033[0m\t" << text << "\n"; \
        exit(1);                                                                 \
    }
#define REB_NOT_IMPLEMENTED(text)                                                          \
    {                                                                                      \
        std::cout << REB_LINE_POS << "\033[31;1mNOT IMPLEMENTED\033[0m\t" << text << "\n"; \
        exit(1);                                                                           \
    }

typedef enum {
    GXX,
    CXX,
    CC,
    SOURCE,
    OBJ,
    BUILD,
    KEEP_OBJ,
    AUTO_RUN,
    IGNORE,
    __count__
} ConfigValue;

auto GetConfigValueFromName(std::string name) -> ConfigValue;

static struct {
    std::string ProgramName;
    std::string Command;
    std::string Flag;
    std::string Section;
    std::string ConfigFilePath = "~/.config/reb/reb.config";
    std::string Config[ConfigValue::__count__] = {""};
} Context;

auto ReadConfig() -> void;
auto GetIgnoreList() -> std::vector<std::string>;

auto Help() -> void;
auto Init(char** argv) -> void;
auto Run(char** argv) -> void;
auto Release(char** argv) -> void;
}  // namespace reb