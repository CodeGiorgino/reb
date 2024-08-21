#pragma once

namespace reb::utils {
#define REB_USAGE()                                                            \
    do {                                                                       \
        std::cout                                                              \
            << std::endl                                                       \
            << "\033[1mDESCRIPTION:\033[0m\n"                                  \
               "reb - Rebuild C/C++ Utility\n\n"                               \
               "\033[1mUSAGE:\033[0m\n"                                        \
               "  reb <command> [parameter]\n"                                 \
               "\n"                                                            \
               "\033[1mCOMMANDS:\033[0m\n"                                     \
               "  help                    Show this message\n\n"               \
               "  init <language>         Initialise the current folder as a " \
               "reb "                                                          \
               "repository\n"                                                  \
               "                          using the provided language\n\n"     \
               "  run <model_name>        Run the model provided\n\n"          \
               "  clean                   Clean the repository\n\n"            \
               "  snap [snapshot_path]    If no parameter is provided, take "  \
               "a \n"                                                          \
               "                          snapshot of the repository, "        \
               "excluding\n"                                                   \
               "                          the file in .rebignore.\n"           \
               "                          If a snapshot_path is provided, "    \
               "revert "                                                       \
               "the repository\n"                                              \
               "                          to the provided snapshot"            \
            << std::endl                                                       \
            << std::endl;                                                      \
    } while (false)

#ifdef DEBUG
#define REB_LINE_POS "\033[4m" << __FILE__ << ":" << __LINE__ << ":\033[0m "
#else
#define REB_LINE_POS ""
#endif

#define REB_INFO(text)                                               \
    do {                                                             \
        std::cout << "\033[33;1mINFO: \033[0m" << text << std::endl; \
    } while (false)

#define REB_ERROR(text)                                                 \
    do {                                                                \
        std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text \
                  << std::endl;                                         \
    } while (false)

#define REB_PANIC(text)                                                 \
    do {                                                                \
        std::cout << REB_LINE_POS << "\033[31;1mERROR: \033[0m" << text \
                  << std::endl;                                         \
        REB_USAGE();                                                    \
        exit(1);                                                        \
    } while (false)

#define REB_NOT_IMPLEMENTED(text)                                         \
    do {                                                                  \
        std::cout << REB_LINE_POS << "\033[31;1mNOT IMPLEMENTED: \033[0m" \
                  << text << "\n";                                        \
        exit(1);                                                          \
    } while (false)
}  // namespace reb::utils
