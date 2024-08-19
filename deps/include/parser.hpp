#pragma once

#include "cppjson.hpp"

namespace json {
namespace parser {
/**
 * @brief Deserialize the json string
 *
 * @param raw The raw json string
 * @return The serialized json node
 */
auto deserialize(std::string raw) -> json_node;
}  // namespace parser
}  // namespace json
