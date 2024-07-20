#pragma once
#include <iostream>
#include <optional>
#include <string>

namespace reb::utils {
#define REB_USAGE                                                              \
    std::cout                                                                  \
        << std::endl                                                           \
        << "\033[1mDESCRIPTION:\033[0m\n"                                      \
           "reb - Rebuild C/C++ Utility\n\n"                                   \
           "\033[1mUSAGE:\033[0m\n"                                            \
           "  reb <command> [parameter]\n"                                     \
           "\n"                                                                \
           "\033[1mCOMMANDS:\033[0m\n"                                         \
           "  help                    Show this message\n\n"                   \
           "  init <language>         Initialise the current folder as a reb " \
           "repository\n"                                                      \
           "                          using the provided language\n\n"         \
           "  run <model_name>        Run the model provided\n\n"              \
           "  snap [snapshot_path]    If no parameter is provided, take a \n"  \
           "                          snapshot of the repository, excluding\n" \
           "                          the file in .rebignore.\n"               \
           "                          If a snapshot_path is provided, revert " \
           "the repository\n"                                                  \
           "                          to the provided snapshot"                \
        << std::endl                                                           \
        << std::endl;

#define REB_LINE_POS "\033[4m" << __FILE__ << ":" << __LINE__ << ":\033[0m "

#define REB_INFO(text) \
    { std::cout << "\033[33;1mINFO: \033[0m" << text << std::endl; }

#define REB_ERROR(text)                                                 \
    {                                                                   \
        std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text \
                  << std::endl;                                         \
    }

#define REB_PANIC(text)                                                 \
    {                                                                   \
        std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text \
                  << std::endl;                                         \
        REB_USAGE                                                       \
        exit(1);                                                        \
    }

#define REB_NOT_IMPLEMENTED(text)                                         \
    {                                                                     \
        std::cout << REB_LINE_POS << "\033[31;1mNOT IMPLEMENTED: \033[0m" \
                  << text << "\n";                                        \
        exit(1);                                                          \
    }
}  // namespace reb::utils

namespace enum_ext {
template <typename T>
auto to_string(const T value) noexcept -> std::string;

template <typename T>
auto from_string(const std::string value) noexcept -> std::optional<T>;
}  // namespace enum_ext
