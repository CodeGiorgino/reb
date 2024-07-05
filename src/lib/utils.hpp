#pragma once

namespace reb::utils {
#define REB_LINE_POS "\033[4m" << __FILE__ << ":" << __LINE__ << ":\033[0m \t"

#define REB_INFO(text)                                                         \
  { std::cout << REB_LINE_POS << "\033[33;1mINFO\033[0m\t" << text << "\n"; }

#define REB_ERROR(text)                                                        \
  { std::cout << REB_LINE_POS << "\033[31;1mERROR\033[0m\t" << text << "\n"; }

#define REB_PANIC(text)                                                        \
  {                                                                            \
    std::cout << REB_LINE_POS << "\033[31;1mERROR\033[0m\t" << text << "\n";   \
    exit(1);                                                                   \
  }

#define REB_NOT_IMPLEMENTED(text)                                              \
  {                                                                            \
    std::cout << REB_LINE_POS << "\033[31;1mNOT IMPLEMENTED\033[0m\t" << text  \
              << "\n";                                                         \
    exit(1);                                                                   \
  }
} // namespace reb::utils
