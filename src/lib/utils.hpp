#pragma once
#include <optional>
#include <string>

namespace reb::utils {
#define REB_LINE_POS "\033[4m" << __FILE__ << ":" << __LINE__ << ":\033[0m "

#define REB_INFO(text) \
    { std::cout << REB_LINE_POS << "\033[33;1mINFO: \033[0m" << text << "\n"; }

#define REB_ERROR(text) \
    { std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text << "\n"; }

#define REB_PANIC(text)                                                 \
    {                                                                   \
        std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text \
                  << "\n";                                              \
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
