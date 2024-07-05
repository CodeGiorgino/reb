#include "config.hpp"

#include <fstream>
#include <iostream>

#include "../context/context.hpp"
#include "../lib/utils.hpp"

namespace reb::config {
auto ReadConfig() -> void {}

auto GetIgnoreList() -> std::vector<std::string> {
    std::vector<std::string> ignoreList{".reb/*"};
    if (reb::context::Context.Config[ConfigValue::IGNORE].empty())
        return ignoreList;

    ignoreList.push_back("");
    for (const auto& ch : reb::context::Context.Config[ConfigValue::IGNORE])
        if (ch == ' ')
            ignoreList.push_back("");
        else
            ignoreList[ignoreList.size() - 1] += ch;

    return ignoreList;
}
}  // namespace reb::config
