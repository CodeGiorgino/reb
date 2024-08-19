#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace json {
/* Forward declaration */
struct json_node;

/* Helper typedefs */
typedef std::pair<std::string, json_node> entry_t;
typedef std::vector<json_node> array_t;
typedef std::unordered_map<std::string, json_node> object_t;
typedef std::variant<void *, bool, int, float, std::shared_ptr<std::string>,
                     std::shared_ptr<array_t>, std::shared_ptr<object_t>>
    json_value;

/* Json definition */
struct json_node final {
    /* Constructors */
    json_node() noexcept;
    json_node(const bool &value) noexcept;
    json_node(const int &value) noexcept;
    json_node(const float &value) noexcept;
    json_node(const char *value) noexcept;
    json_node(const std::string &value) noexcept;
    json_node(const array_t &value) noexcept;
    json_node(const object_t &value) noexcept;
    json_node(const json_node &node) noexcept;

    ~json_node() noexcept = default;

    /* Type cast overload */
    operator bool() const;
    operator int() const;
    operator float() const;
    operator std::string() const;
    operator array_t() const;
    operator object_t() const;

    /**
     * @brief Access the specified object node
     *
     * @param key The key of the node
     * @return The node reference
     */
    auto operator[](const size_t &index) -> json_node &;
    auto operator[](const std::string &key) -> json_node &;

    /**
     * @brief Append the given node to an array_t
     *
     * @param node The node to append
     * @return The appended node
     */
    auto operator<<(const json_node &node) -> json_node &;

    /**
     * @brief Emplace the given node to an object_t
     *
     * @param entry The entry to emplace
     * @return The emplaced node
     */
    auto operator<<(const entry_t &entry) -> json_node &;

    auto operator=(const json_node &node) noexcept -> json_node &;
    auto operator=(const bool &value) noexcept -> json_node &;
    auto operator=(const int &value) noexcept -> json_node &;
    auto operator=(const float &value) noexcept -> json_node &;
    auto operator=(const char *value) noexcept -> json_node &;
    auto operator=(const std::string &value) noexcept -> json_node &;
    auto operator=(const array_t &value) noexcept -> json_node &;
    auto operator=(const object_t &value) noexcept -> json_node &;

    /* Function members */
    /**
     * @brief Dump the node
     *
     * @param indent The indentation level to follow (if 0 is provided, print on
     * one line)
     * @return The serialized node
     */
    auto dump(size_t indent) const noexcept -> std::string;

    /**
     * @brief Set the object to hold a null value
     *
     * @return The node reference
     */
    auto set_null() noexcept -> json_node &;

    /**
     * @brief Check if the value is set to null
     *
     * @return If the value is set to null
     */
    auto is_null() const noexcept -> bool;

   private:
    /* The node value */
    json_value _value;
};
}  // namespace json
