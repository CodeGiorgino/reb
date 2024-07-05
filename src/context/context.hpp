#pragma once
#include <string>
#include "../config/config.hpp"

namespace reb::context {
static struct {
  std::string ProgramName;
  std::string Command;
  std::string Flag;
  std::string Section;
  std::string ConfigFilePath = std::string(std::getenv("HOME")) + "/.config/reb/reb.config";
  std::string Config[reb::config::ConfigValue::__count__] = {""};
} Context;
}
