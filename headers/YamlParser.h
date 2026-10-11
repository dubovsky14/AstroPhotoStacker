#pragma once

#include <vector>
#include <string>
#include <map>
#include <utility>

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

    class YamlNode {
        public:
            YamlNode() = default;

            //explicit YamlNode(const std::string &yaml_address);

            YamlNode(const std::vector<std::string>& valid_lines);

            YamlNodeDataType get_data_type() const { return m_data_type; };

            const std::map<std::string, YamlNode>& get_map() const;

            const std::vector<YamlNode>& get_list() const;

            const std::string& get_string() const;

            int get_int() const;

            float get_float() const;

            bool get_bool() const;

            YamlNode& operator[](const std::string &key);

            YamlNode& operator[](size_t index);

            const YamlNode& operator[](const std::string &key) const;

            const YamlNode& operator[](size_t index) const;

            YamlNode& operator= (const std::string& value);

            YamlNode& operator= (int value);

            YamlNode& operator= (float value);

            YamlNode& operator= (bool value);

            std::string to_string(const std::string &initial_indentation = "") const;

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

    };

    YamlNode load_yaml_file(const std::string &yaml_address);

}