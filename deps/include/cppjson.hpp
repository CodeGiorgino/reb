#pragma once

#include <compare>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace json {
/* Forward declaration */
struct json_node;

/* Helper typedefs */
using entry_t = std::pair<std::string, json_node>;
using array_t = std::vector<json_node>;
using object_t = std::unordered_map<std::string, json_node>;
using json_value =
    std::variant<void *, bool, int, float, std::shared_ptr<std::string>,
                 std::shared_ptr<array_t>, std::shared_ptr<object_t>>;

/* Json definition */
struct json_node final {
   public:
    /* Constructor */
    json_node() noexcept;

    /* Copy constructor */
    json_node(const bool &value) noexcept;
    json_node(const int &value) noexcept;
    json_node(const float &value) noexcept;
    json_node(const std::string &value) noexcept;
    json_node(const array_t &value) noexcept;
    json_node(const object_t &value) noexcept;

    json_node(const json_node &other) noexcept = default;
    auto operator=(const json_node &other) noexcept -> json_node & = default;

    /* Move constructor */
    json_node(bool &&value) noexcept;
    json_node(int &&value) noexcept;
    json_node(float &&value) noexcept;
    json_node(std::string &&value) noexcept;
    json_node(array_t &&value) noexcept;
    json_node(object_t &&value) noexcept;

    json_node(json_node &&other) noexcept = default;
    auto operator=(json_node &&other) noexcept -> json_node & = default;

    /* Type cast overload */
   public:
    template <typename Tp>
    operator Tp() const {
        if (auto value = try_get_value<Tp>(); value.has_value())
            return value.value();
        throw std::bad_cast();
    }

   public:
    /**
     * @brief Access the specified object node
     *
     * @param key The index of the node
     * @return The node reference
     */
    auto operator[](const size_t &index) -> json_node &;

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

    /* Function members */
   public:
    /**
     * @brief Dump the node
     *
     * @param indent The indentation to follow (if 0 is provided, print on
     * one line)
     * @param level The indentation level
     * @return The serialized node
     */
    auto dump(size_t indent, size_t level = 0) const noexcept -> std::string;

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

    /**
     * @brief Try to get the json_node value
     *
     * @tparam Tp The value to get
     * @return The json_node value or std::nullopt
     */
    template <typename Tp>
    auto try_get_value() const noexcept -> std::optional<Tp> {
        static_assert(std::__detail::__variant::__exactly_once<
                          Tp, bool, int, float, std::string, array_t, object_t>,
                      "T must occur exactly once in json_value alternatives");

        if constexpr (std::__detail::__variant::__exactly_once<Tp, bool, int,
                                                               float>)
            return std::get<Tp>(_value);
        else
            return *std::get<std::shared_ptr<Tp>>(_value);

        return std::nullopt;
    }

   private:
    /* The node value */
    json_value _value;
};
}  // namespace json
