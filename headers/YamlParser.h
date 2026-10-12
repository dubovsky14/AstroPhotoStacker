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

    class YamlNode {
        public:
            YamlNode() = default;

            YamlNode(const std::vector<std::string>& valid_lines);

            YamlNodeDataType get_data_type() const { return m_data_type; };

            const std::map<std::string, YamlNode>& get_map() const;

            const std::vector<YamlNode>& get_list() const;

            template<typename ValueType>
            ValueType get_value() const;

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
            YamlNode& operator[](const std::string &key);

            YamlNode& operator[](size_t index);

            const YamlNode& operator[](const std::string &key) const;

            const YamlNode& operator[](size_t index) const;

            YamlNode& operator= (const std::string& value);

            YamlNode& operator= (int value);

            YamlNode& operator= (float value);

            YamlNode& operator= (bool value);

            std::string to_string(const std::string &initial_indentation = "") const;

            size_t size() const;

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
            static const std::string OUTPUT_INDENTATION_LIST;

            void clean_up();

            // @brief Get the indentation length with a hypen and without it
            static std::pair<size_t, size_t> get_indentation_length(const std::string &line);

            static bool remove_matching_list_brackets(std::string *input_string, bool throw_on_mismatch = true);

            static std::vector<std::string> parse_list_elements(const std::string &list_string);

            static std::pair<std::string,std::string> split_key_value(const std::string &line);

            template<typename ValueType>
            bool matches_type() const;

    };

    YamlNode load_yaml_file(const std::string &yaml_address);

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


}