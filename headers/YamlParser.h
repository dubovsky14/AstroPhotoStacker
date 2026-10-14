#pragma once

#include <vector>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <optional>

namespace AstroPhotoStacker {
    enum class YamlNodeDataType {
        EMPTY,
        MAP,
        LIST,
        STRING,
        INT,
        FLOAT,
        BOOL
    };

    std::ostream& operator <<(std::ostream &os, const YamlNodeDataType &node_type);

    /**
     * @brief Represents a node in a YAML document, which can be a map, list, or scalar value.
     * This class provides methods to access the node's data as a map, list, or scalar value, and to query its type.
     *
     * @note To parse a YAML document, use the `load_yaml_file` function to obtain the root `YamlNode`.
     */
    class YamlNode {
        public:
            YamlNode() = default;

            YamlNode(const std::vector<std::string>& valid_lines);

            /**
             * @brief Get the data type of the YAML node (dict, list, float, int, string, etc.).
             * @return The data type of the node.
             */
            YamlNodeDataType get_data_type() const { return m_data_type; };

            /**
             * @brief Get the map representation of the YAML node.
             * @return The map of key-value pairs if the node is a map.
             * @throws std::runtime_error if the node is not a map.
             */
            const std::map<std::string, YamlNode>& get_map() const;

            /**
             * @brief Get the list representation of the YAML node.
             * @return The list of YAML nodes if the node is a list.
             * @throws std::runtime_error if the node is not a list.
             */
            const std::vector<YamlNode>& get_list() const;

            /**
             * @brief Get the value of the YAML node as the specified type.
             * @tparam ValueType The type to convert the node's value to.
             * @return The value of the node as the specified type.
             * @throws std::runtime_error if the node's type does not match the requested type and conversion is not possible.
             */
            template<typename ValueType>
            ValueType get_value() const;

            /**
             * @brief Convenience function to get the value of a YAML node at a specific index in a list, or return default if not present.
             * @tparam ValueType The type to convert the node's value to.
             * @param index The index of the element in the list.
             * @param default_value The default value to return if the index is out of bounds.
             * @return The value of the node at the specified index as the specified type.
             * @throws std::runtime_error if the node is not a list, the type does not match or index is out of bounds and no default value is provided.
             */
            template<typename ValueType>
            ValueType get_value_at_index(const size_t &index, const std::optional<ValueType> &default_value = std::nullopt) const  {
                if (m_data_type != YamlNodeDataType::LIST) {
                    throw std::runtime_error("Node is not a list");
                }

                if (index >= m_list.size()) {
                    if (!default_value.has_value()) {
                        throw std::runtime_error("Index " + std::to_string(index) + " out of bounds and no default value provided");
                    }
                    return default_value.value();
                }
                const YamlNode &node = m_list[index];
                if (!node.matches_type<ValueType>()) {
                    throw std::runtime_error("Type mismatch at index " + std::to_string(index));
                }
                return node.get_value<ValueType>();
            };

            /**
             * @brief Convenience function to get the value of a YAML node at a specific key in dictionary, or return default if not present.
             * @tparam ValueType The type to convert the node's value to.
             * @param key The key of the element in the dictionary.
             * @param default_value The default value to return if the key is not present.
             * @return The value of the node at the specified key as the specified type.
             * @throws std::runtime_error if the node is not a map, the type does not match or the key is not present and no default value is provided.
             */
            template<typename ValueType>
            ValueType get_value_at_key(const std::string &key, const std::optional<ValueType> &default_value = std::nullopt) const  {
                if (m_data_type != YamlNodeDataType::MAP) {
                    throw std::runtime_error("Node is not a map");
                }
                if (m_map.find(key) == m_map.end()) {
                    if (!default_value.has_value()) {
                        throw std::runtime_error("Key " + key + " not found and no default value provided");
                    }
                    return default_value.value();
                }
                const YamlNode &node = m_map.at(key);
                if (!node.matches_type<ValueType>()) {
                    throw std::runtime_error("Type mismatch for key " + key);
                }
                return node.get_value<ValueType>();
            };

            /**
             * @brief Access a YAML node by key in a dictionary.
             * @param key The key of the element in the dictionary.
             * @return A reference to the YAML node at the specified key.
             * @throws std::runtime_error if the node is not a map or the key is not present.
             */
            YamlNode& operator[](const std::string &key);

            /**
             * @brief Access a YAML node by index in a list.
             * @param index The index of the element in the list.
             * @return A reference to the YAML node at the specified index.
             * @throws std::runtime_error if the node is not a list or the index is out of bounds.
             */
            YamlNode& operator[](size_t index);

            /**
             * @brief Access a YAML node by key in a dictionary.
             * @param key The key of the element in the dictionary.
             * @return A reference to the YAML node at the specified key.
             * @throws std::runtime_error if the node is not a map or the key is not present.
             */
            const YamlNode& operator[](const std::string &key) const;

            /**
             * @brief Access a YAML node by index in a list.
             * @param index The index of the element in the list.
             * @return A reference to the YAML node at the specified index.
             * @throws std::runtime_error if the node is not a list or the index is out of bounds.
             */
            const YamlNode& operator[](size_t index) const;

            /**
             * @brief Assign a string value to the YAML node.
             * @param value The string value to assign.
             * @return A reference to the YAML node.
             */
            YamlNode& operator= (const std::string& value);
            /**
             * @brief Assign an integer value to the YAML node.
             * @param value The integer value to assign.
             * @return A reference to the YAML node.
             */
            YamlNode& operator= (int value);

            /**
             * @brief Assign a float value to the YAML node.
             * @param value The float value to assign.
             * @return A reference to the YAML node.
             */
            YamlNode& operator= (float value);

            /**
             * @brief Assign a boolean value to the YAML node.
             * @param value The boolean value to assign.
             * @return A reference to the YAML node.
             */
            YamlNode& operator= (bool value);

            /**
             * @brief Convert the YAML node to a string representation.
             * @param initial_indentation The initial indentation to use for the string representation.
             * @return The string representation of the YAML node.
             */
            std::string to_string(const std::string &initial_indentation = "") const;

            /**
             * @brief Get the number of elements in the YAML node if it is a list or a map.
             * @return The number of elements in the YAML node.
             * @throws std::runtime_error if the node is not a list or a map.
             */
            size_t size() const;

            /**
             * @brief Check if the YAML node has a specific key if it is a map.
             * @param key The key to check for.
             * @return True if the key exists, false otherwise.
             * @throws std::runtime_error if the node is not a map.
             */
            bool has_key(const std::string &key) const;

        private:
            std::map<std::string, YamlNode> m_map;
            std::vector<YamlNode>           m_list;
            std::string                     m_value_string;
            int                             m_value_int;
            float                           m_value_float;
            bool                            m_value_bool;

            YamlNodeDataType                m_data_type = YamlNodeDataType::EMPTY;


            static const std::string OUTPUT_INDENTATION;

            void clean_up();

            // @brief Get the indentation length with a hypen and without it
            static std::pair<size_t, size_t> get_indentation_length(const std::string &line);

            static bool remove_matching_list_brackets(std::string *input_string, bool throw_on_mismatch = true);

            static std::vector<std::string> parse_list_elements(const std::string &list_string);

            static std::pair<std::string,std::string> split_key_value(const std::string &line);

            template<typename ValueType>
            bool matches_type() const;

            bool is_multiline_block() const;

    };

    template<>
    int YamlNode::get_value<int>() const;

    template<>
    float YamlNode::get_value<float>() const;

    template<>
    bool YamlNode::get_value<bool>() const;

    template<>
    std::string YamlNode::get_value<std::string>() const;


    template<> inline
    bool YamlNode::matches_type<int>() const { return m_data_type == YamlNodeDataType::INT || m_data_type == YamlNodeDataType::FLOAT; };

    template<> inline
    bool YamlNode::matches_type<float>() const { return m_data_type == YamlNodeDataType::FLOAT || m_data_type == YamlNodeDataType::INT; };

    template<> inline
    bool YamlNode::matches_type<bool>() const { return m_data_type == YamlNodeDataType::BOOL; };

    template<> inline
    bool YamlNode::matches_type<std::string>() const { return m_data_type == YamlNodeDataType::STRING; };


    /**
     * @brief Load a YAML file and return its root node.
     * @param yaml_address The file path to the YAML file.
     * @return The root YamlNode of the loaded YAML file.
     */
    YamlNode load_yaml_file(const std::string &yaml_address);

}