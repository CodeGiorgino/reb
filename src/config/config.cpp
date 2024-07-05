#include "config.hpp"
#include "../context/context.hpp"
#include "../lib/utils.hpp"
#include <fstream>
#include <iostream>

namespace reb::config {
auto ReadConfig() -> void {
  std::ifstream configFile(reb::context::Context.ConfigFilePath);
  if (!configFile.is_open())
    REB_PANIC("Cannot open config file: '" + reb::context::Context.ConfigFilePath +
              "'")

  std::string line;
  while (getline(configFile, line)) {
    if (line.empty() || line.starts_with("#"))
      continue;
    else if (line.starts_with("- ") &&
             line.substr(2) == (reb::context::Context.Section + ":")) {
      while (getline(configFile, line)) {
        if (line.empty() || line.starts_with("#"))
          continue;
        else if (line.starts_with("- "))
          break;
        else if (!line.starts_with("    - "))
          REB_PANIC("Invalid line in config file section '" +
                    reb::context::Context.Section + "': '" + line + "'")

        if (line.find('"') == std::string::npos ||
            line.find_first_of('"') == line.find_last_of("'") ||
            !line.ends_with('"'))
          REB_PANIC("Invalid line in config file section '" +
                    reb::context::Context.Section + "': '" + line + "'")

        auto configValueName = line.substr(6, line.find_first_of('=') - 6);
        configValueName.erase(configValueName.begin() +
                                  configValueName.find_first_of(' '),
                              configValueName.end());

        const auto configValue =
            enum_ext::from_string<ConfigValue>(configValueName);
        if (!configValue.has_value())
          REB_PANIC("unknown config value name in section " +
                    reb::context::Context.Section + ": " + configValueName)

        reb::context::Context.Config[configValue.value()] =
            line.substr(line.find_first_of('"') + 1);
        reb::context::Context.Config[configValue.value()].erase(
            reb::context::Context.Config[configValue.value()].length() - 1);
      };
    }
  };

  configFile.close();
}

auto GetIgnoreList() -> std::vector<std::string> {
  std::vector<std::string> ignoreList = {".reb", ""};
  for (const auto &ch : reb::context::Context.Config[ConfigValue::IGNORE])
    if (ch == ' ')
      ignoreList.push_back("");
    else
      ignoreList[ignoreList.size() - 1] += ch;

  return ignoreList;
}
} // namespace reb::config
