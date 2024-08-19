#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace json {
/* Forward declaration */
struct json_node;

/* Helper typedefs */
typedef std::vector<json_node> array_t;
typedef std::pair<std::string, json_node> entry_t;
typedef std::vector<entry_t> object_t;

/* Json definition */
struct json_node final {
    /* Constructors */
    json_node() noexcept;
    json_node(bool value) noexcept;
    json_node(int value) noexcept;
    json_node(float value) noexcept;
    json_node(const char *value) noexcept;
    json_node(std::string value) noexcept;
    json_node(array_t value) noexcept;
    json_node(object_t value) noexcept;
    json_node(const json_node &node) noexcept;

    ~json_node() = default;

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
    auto operator[](const char *key) -> json_node &;
    /**
     * @brief Access the specified object node
     *
     * @param key The key of the node
     * @return The node reference
     */
    auto operator[](std::string key) -> json_node &;

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
     * @brief Check if the value is set to null
     *
     * @return If the value is set to null
     */
    auto is_null() const noexcept -> bool;

   private:
    /* Json value types */
    enum class enum_value_type : size_t {
        json_nil,
        json_bool,
        json_int,
        json_float,
        json_string,
        json_array,
        json_object
    };

    /* The current value type */
    enum_value_type _type;

    /* The node values */
    void *_value_nil{nullptr};
    bool _value_bool{};
    int _value_int{};
    float _value_float{};
    std::string _value_string{};
    array_t _value_array{};
    object_t _value_object{};
};
}  // namespace json
