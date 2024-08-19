#pragma once

#include "main.hpp"

namespace json {
namespace parser {
auto deserialize(std::string raw) -> json_node;
}  // namespace parser
}  // namespace json
