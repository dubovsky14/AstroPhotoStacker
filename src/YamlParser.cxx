#include "../headers/YamlParser.h"

#include "../headers/Common.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


const std::string YamlNode::OUTPUT_INDENTATION = "  ";


YamlNode AstroPhotoStacker::load_yaml_file(const std::string &yaml_address) {
    std::ifstream file(yaml_address);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open YAML file: " + yaml_address);
    }
    std::vector<std::string> valid_lines;
    std::string line;
    while (std::getline(file, line)) {
        const size_t first_non_whitespace = line.find_first_not_of(" \t");
        const size_t first_hash = line.find('#');
        if (first_non_whitespace == std::string::npos) {
            continue;
        }
        if (first_hash != std::string::npos && first_hash < first_non_whitespace)
            line = line.substr(0, first_hash);
        valid_lines.push_back(line);
    }
    file.close();
    return YamlNode(valid_lines);
}

YamlNode::YamlNode(const std::vector<std::string>& valid_lines) {
    if (valid_lines.empty()) {
        return;
    }

    // in this case, the line is only the value, so we just need to get its type, convert it and store it
    if (valid_lines.size() == 1) {
        if (string_is_int(valid_lines[0])) {
            m_data_type = YamlNodeDataType::INT;
            m_value_int = std::stoi(valid_lines[0]);
        } else if (string_is_float(valid_lines[0])) {
            m_data_type = YamlNodeDataType::FLOAT;
            m_value_float = std::stof(valid_lines[0]);
        } else {
            m_data_type = YamlNodeDataType::STRING;
            std::string first_line = valid_lines[0];
            strip_string(&first_line, "\"'");
            m_value_string = first_line;
        }
        return;
    }


    const auto [initial_indentation_length, initial_indentation_wo_hypen] = get_indentation_length(valid_lines[0]);
    bool is_list = (initial_indentation_wo_hypen < initial_indentation_length);
    if (is_list) {
        m_data_type = YamlNodeDataType::LIST;
    }
    else {
        m_data_type = YamlNodeDataType::MAP;
    }

    vector<string> nested_block_lines;
    vector<string> current_list_element;
    bool reading_nested_block = false;
    string previous_key = "";
    for (const std::string &line : valid_lines) {
        if (line.size() <= initial_indentation_length) {
            throw std::runtime_error("Line is shorter than initial indentation");
        }

        const auto [this_line_indentation_length, this_indentation_wo_hypen] = get_indentation_length(line);

        if (this_line_indentation_length > initial_indentation_length) {
            reading_nested_block = true;
            nested_block_lines.push_back(line);
            continue;
        }
        if (reading_nested_block && (this_line_indentation_length == initial_indentation_length)) {
            reading_nested_block = false;

            if (previous_key.empty() && !is_list) {
                throw std::runtime_error("Previous key is empty while reading nested block on line: " + line);
            }
            if (!previous_key.empty() && !nested_block_lines.empty()) {
                throw std::runtime_error("Found invalid nested block: " + line);
            }
            if (previous_key.size() != 0)   {
                m_map[previous_key] = YamlNode(nested_block_lines);
            }
            else if (is_list) {
                m_list.push_back(YamlNode(nested_block_lines));
            }
            nested_block_lines.clear();
        }

        // now we can read the line assuming we are not reading a nested block
        const bool this_line_starts_new_list_element = (this_indentation_wo_hypen < this_line_indentation_length);
        if (is_list) {
            if (this_line_starts_new_list_element) {
                if (!current_list_element.empty()) {
                    m_list.push_back(YamlNode(current_list_element));
                    current_list_element.clear();
                }
                string line_copy = line;
                line_copy[this_indentation_wo_hypen] = ' ';
                current_list_element.push_back(line_copy);
            }
            else {
                current_list_element.push_back(line);
            }
        }
        else {
            const string trimmed_line = line.substr(this_indentation_wo_hypen);
            auto colon_pos = trimmed_line.find(':');
            if (colon_pos == std::string::npos) {
                throw std::runtime_error("Invalid line format: " + line);
            }
            previous_key = trimmed_line.substr(0, colon_pos);
            std::string value = trimmed_line.substr(colon_pos + 1);
            strip_string(&value, " \t");
            if (!value.empty()) {
                m_map[previous_key] = YamlNode({value});
            }
        }
    }

    if (is_list && !current_list_element.empty()) {
        m_list.push_back(YamlNode(current_list_element));
        current_list_element.clear();
    }
    if (!previous_key.empty() && !nested_block_lines.empty()) {
        m_map[previous_key] = YamlNode(nested_block_lines);
        nested_block_lines.clear();
    }
}

const std::map<std::string, YamlNode>& YamlNode::get_map() const {
    if (this->get_data_type() != YamlNodeDataType::MAP) {
        throw std::runtime_error("YamlNode is not a map");
    }
    return m_map;
}

const std::vector<YamlNode>& YamlNode::get_list() const {
    if (this->get_data_type() != YamlNodeDataType::LIST) {
        throw std::runtime_error("YamlNode is not a list");
    }
    return m_list;
}

const std::string &YamlNode::get_string() const {
    if (this->get_data_type() != YamlNodeDataType::STRING) {
        throw std::runtime_error("YamlNode is not a string");
    }
    return m_value_string;
}

int YamlNode::get_int() const {
    if (this->get_data_type() != YamlNodeDataType::INT) {
        throw std::runtime_error("YamlNode is not an int");
    }
    return m_value_int;
}

float YamlNode::get_float() const {
    if (this->get_data_type() != YamlNodeDataType::FLOAT) {
        throw std::runtime_error("YamlNode is not a float");
    }
    return m_value_float;
}

bool YamlNode::get_bool() const {
    if (this->get_data_type() != YamlNodeDataType::BOOL) {
        throw std::runtime_error("YamlNode is not a bool");
    }
    return m_value_bool;
}

YamlNode& YamlNode::operator[](const std::string &key) {
    if (this->get_data_type() != YamlNodeDataType::MAP) {
        throw std::runtime_error("YamlNode is not a map");
    }
    if (m_map.find(key) == m_map.end()) {
        throw std::runtime_error("Key not found in YamlNode map");
    }
    return m_map.at(key);
}

YamlNode& YamlNode::operator[](size_t index) {
    if (this->get_data_type() != YamlNodeDataType::LIST) {
        throw std::runtime_error("YamlNode is not a list");
    }
    if (index >= m_list.size()) {
        throw std::runtime_error("Index out of bounds in YamlNode list");
    }
    return m_list.at(index);
}
const YamlNode& YamlNode::operator[](const std::string &key) const {
    if (this->get_data_type() != YamlNodeDataType::MAP) {
        throw std::runtime_error("YamlNode is not a map");
    }
    if (m_map.find(key) == m_map.end()) {
        throw std::runtime_error("Key not found in YamlNode map");
    }
    return m_map.at(key);
}

const YamlNode& YamlNode::operator[](size_t index) const {
    if (this->get_data_type() != YamlNodeDataType::LIST) {
        throw std::runtime_error("YamlNode is not a list");
    }
    if (index >= m_list.size()) {
        throw std::runtime_error("Index out of bounds in YamlNode list");
    }
    return m_list.at(index);
}

YamlNode& YamlNode::operator= (const std::string& value) {
    clean_up();
    m_value_string = value;
    m_data_type = YamlNodeDataType::STRING;
    return *this;
}

YamlNode& YamlNode::operator= (int value) {
    clean_up();
    m_value_int = value;
    m_data_type = YamlNodeDataType::INT;
    return *this;
}

YamlNode& YamlNode::operator= (float value) {
    clean_up();
    m_value_float = value;
    m_data_type = YamlNodeDataType::FLOAT;
    return *this;
}

YamlNode& YamlNode::operator= (bool value) {
    clean_up();
    m_value_bool = value;
    m_data_type = YamlNodeDataType::BOOL;
    return *this;
}

std::string YamlNode::to_string(const std::string &initial_indentation) const {
    if (this->get_data_type() == YamlNodeDataType::STRING) {
        return initial_indentation + "\"" + m_value_string + "\"";
    }
    if (this->get_data_type() == YamlNodeDataType::INT) {
        return initial_indentation + std::to_string(m_value_int);
    }
    if (this->get_data_type() == YamlNodeDataType::FLOAT) {
        return initial_indentation + std::to_string(m_value_float);
    }
    if (this->get_data_type() == YamlNodeDataType::BOOL) {
        return initial_indentation + (m_value_bool ? "True" : "False");
    }
    if (this->get_data_type() == YamlNodeDataType::MAP) {
        std::string result = "\n";
        for (const auto& [key, value] : m_map) {
            result += initial_indentation + OUTPUT_INDENTATION + key + ": " + value.to_string(initial_indentation + OUTPUT_INDENTATION) + "\n";
        }
        return result;
    }
    if (this->get_data_type() == YamlNodeDataType::LIST) {
        std::string result = "\n";
        for (const auto& value : m_list) {
            result += value.to_string(initial_indentation + OUTPUT_INDENTATION) + "\n";
        }
        return result;
    }
    return "";
}






void YamlNode::clean_up() {
    m_map.clear();
    m_list.clear();
    m_value_string.clear();
    m_value_int = 0;
    m_value_float = 0.0f;
    m_value_bool = false;
    m_data_type = YamlNodeDataType::EMPTY;
}


std::pair<size_t, size_t> YamlNode::get_indentation_length(const std::string &line) {
    size_t indentation_length = 0;

    int indentation_before_hypen = -1;
    for (char c : line) {
        if (c == ' ') {
            ++indentation_length;
        } else if (c == '\t') {
            ++indentation_length;
        } else if (c == '-') {
            indentation_before_hypen = indentation_length;
            ++indentation_length;
            break;
        } else {
            break;
        }
    }

    if (indentation_before_hypen == -1) {
        indentation_before_hypen = indentation_length;
    }
    size_t indentation_before_hypen_size_t = static_cast<size_t>(indentation_before_hypen);
    return {indentation_length, indentation_before_hypen_size_t};
}