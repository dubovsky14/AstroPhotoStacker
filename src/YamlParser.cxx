#include "../headers/YamlParser.h"

#include "../headers/Common.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


const std::string YamlNode::OUTPUT_INDENTATION = "  ";


std::ostream& AstroPhotoStacker::operator <<(std::ostream &os, const YamlNodeDataType &node_type) {
    switch (node_type) {
        case YamlNodeDataType::EMPTY: os << "EMPTY"; break;
        case YamlNodeDataType::MAP: os << "MAP"; break;
        case YamlNodeDataType::LIST: os << "LIST"; break;
        case YamlNodeDataType::STRING: os << "STRING"; break;
        case YamlNodeDataType::INT: os << "INT"; break;
        case YamlNodeDataType::FLOAT: os << "FLOAT"; break;
        case YamlNodeDataType::BOOL: os << "BOOL"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}


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
        if (first_hash <= first_non_whitespace) {
            continue;
        }
        if (first_hash != std::string::npos && first_hash > first_non_whitespace)   {
            int n_double_quotes(0), n_single_quotes(0);
            for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '"') ++n_double_quotes;
                if (line[i] == '\'') ++n_single_quotes;
                if (n_double_quotes % 2 == 0 && n_single_quotes % 2 == 0 && line[i] == '#') {
                    line = line.substr(0, i);
                    break;
                }
            }
        }
        valid_lines.push_back(line);
    }
    file.close();
    return YamlNode(valid_lines);
}

// #TODO:
// 1. One line maps
// 2. better error messages

YamlNode::YamlNode(const std::vector<std::string>& valid_lines) {
    if (valid_lines.empty()) {
        return;
    }

    // in this case, the line is only the value, so we just need to get its type, convert it and store it
    if (valid_lines.size() == 1) {
        std::string first_line = valid_lines[0];
        const bool had_quotation_marks = strip_string_and_remove_quotation_marks(&first_line);
        if (had_quotation_marks) {
            m_data_type = YamlNodeDataType::STRING;
            m_value_string = first_line;
            return;
        }
        else if (remove_matching_list_brackets(&first_line, true, {{'[', ']'}})) {
            m_data_type = YamlNodeDataType::LIST;

            // parsing the list elements from the string
            std::vector<std::string> list_elements = get_elements_of_nested_structure(first_line);
            for (const std::string &element : list_elements) {
                m_list.push_back(YamlNode({element}));
            }
        }
        else if (remove_matching_list_brackets(&first_line, true, {{'{', '}'}})) {
            m_data_type = YamlNodeDataType::MAP;
            const vector<string> map_elements = get_elements_of_nested_structure(first_line);
            for (const std::string &element : map_elements) {
                const std::pair<string,string> key_value = split_key_value(element);
                if (!key_value.first.empty()) {
                    m_map[key_value.first] = YamlNode({key_value.second});
                }
            }
        }
        else if (string_is_int(first_line)) {
            m_data_type = YamlNodeDataType::INT;
            m_value_int = std::stoi(first_line);
        }
        else if (string_is_float(first_line)) {
            m_data_type = YamlNodeDataType::FLOAT;
            m_value_float = std::stof(first_line);
        }
        else if (compare_case_insensitive(first_line, "true") || compare_case_insensitive(first_line, "false")) {
            m_data_type = YamlNodeDataType::BOOL;
            m_value_bool = compare_case_insensitive(first_line, "true");
        }
        else {
            const std::pair<string,string> key_value = split_key_value(first_line);
            if (!key_value.first.empty()) {
                m_data_type = YamlNodeDataType::MAP;
                m_map[key_value.first] = YamlNode({key_value.second});
                return;
            }

            size_t first_non_whitespace = first_line.find_first_not_of(" \t");
            if (first_non_whitespace != std::string::npos && first_line[first_non_whitespace] == '-') {
                m_data_type = YamlNodeDataType::LIST;
                first_line[first_non_whitespace] = ' ';
                strip_string_and_remove_quotation_marks(&first_line);
                m_list.push_back(YamlNode({first_line}));
                return;
            }

            m_data_type = YamlNodeDataType::STRING;
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

        const bool this_line_starts_new_list_element = (this_indentation_wo_hypen < this_line_indentation_length) && is_list && (this_line_indentation_length == initial_indentation_length);
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
            continue;
        }

        if (this_line_indentation_length > initial_indentation_length) {
            reading_nested_block = true;
            nested_block_lines.push_back(line.substr(initial_indentation_length));
            continue;
        }
        if (reading_nested_block && (this_line_indentation_length == initial_indentation_length)) {
            reading_nested_block = false;

            if (previous_key.empty() && !is_list) {
                throw std::runtime_error("Previous key is empty while reading nested block on line: " + line);
            }
            if (previous_key.empty() && !nested_block_lines.empty() && !is_list) {
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

        if (!is_list) {
            const string trimmed_line = line.substr(this_indentation_wo_hypen);

            auto [key, value] = split_key_value(line);
            if (key.empty()) {
                throw std::runtime_error("Invalid line format: " + line);
            }
            previous_key = key;
            if (!value.empty()) {
                m_map[previous_key] = YamlNode({value});
            }
        }
    }

    if (is_list && !current_list_element.empty()) {
        m_list.push_back(YamlNode(current_list_element));
    }
    if (!previous_key.empty() && !nested_block_lines.empty()) {
        m_map[previous_key] = YamlNode(nested_block_lines);
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

template<>
int YamlNode::get_value<int>() const {
    if (this->get_data_type() == YamlNodeDataType::INT) {
        return m_value_int;
    }
    if (this->get_data_type() == YamlNodeDataType::FLOAT) {
        return static_cast<int>(m_value_float);
    }
    throw std::runtime_error("YamlNode is not an int");
}

template<>
float YamlNode::get_value<float>() const {
    if (this->get_data_type() == YamlNodeDataType::FLOAT) {
        return m_value_float;
    }
    if (this->get_data_type() == YamlNodeDataType::INT) {
        return static_cast<float>(m_value_int);
    }
    throw std::runtime_error("YamlNode is not a float");
}

template<>
bool YamlNode::get_value<bool>() const {
    if (this->get_data_type() != YamlNodeDataType::BOOL) {
        throw std::runtime_error("YamlNode is not a bool");
    }
    return m_value_bool;
}

template<>
std::string YamlNode::get_value<std::string>() const {
    if (this->get_data_type() != YamlNodeDataType::STRING) {
        throw std::runtime_error("YamlNode is not a string");
    }
    return m_value_string;
}


YamlNode& YamlNode::operator[](const std::string &key) {
    if (this->get_data_type() != YamlNodeDataType::MAP) {
        throw std::runtime_error("YamlNode is not a map");
    }
    if (m_map.find(key) == m_map.end()) {
        throw std::runtime_error("Key not found in YamlNode map: " + key);
    }
    return m_map.at(key);
}

YamlNode& YamlNode::operator[](size_t index) {
    if (this->get_data_type() != YamlNodeDataType::LIST) {
        throw std::runtime_error("YamlNode is not a list");
    }
    if (index >= m_list.size()) {
        throw std::runtime_error("Index out of bounds in YamlNode list: " + std::to_string(index));
    }
    return m_list.at(index);
}
const YamlNode& YamlNode::operator[](const std::string &key) const {
    if (this->get_data_type() != YamlNodeDataType::MAP) {
        throw std::runtime_error("YamlNode is not a map");
    }
    if (m_map.find(key) == m_map.end()) {
        throw std::runtime_error("Key not found in YamlNode map: " + key);
    }
    return m_map.at(key);
}

const YamlNode& YamlNode::operator[](size_t index) const {
    if (this->get_data_type() != YamlNodeDataType::LIST) {
        throw std::runtime_error("YamlNode is not a list");
    }
    if (index >= m_list.size()) {
        throw std::runtime_error("Index out of bounds in YamlNode list: " + std::to_string(index));
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
        std::string result = "";
        for (const auto& [key, value] : m_map) {
            result += initial_indentation + key + ": "
            + (value.is_multiline_block() ? "\n" : "")
            + value.to_string(value.is_multiline_block() ? (initial_indentation + OUTPUT_INDENTATION) : "") + "\n";
        }
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        return result;
    }
    if (this->get_data_type() == YamlNodeDataType::LIST) {
        std::string result = "";
        for (const auto& value : m_list) {
            string list_string = value.to_string(initial_indentation + OUTPUT_INDENTATION);
            list_string[(initial_indentation + OUTPUT_INDENTATION).size()-2] = '-';
            result += list_string + "\n";
        }

        result.resize(result.size() - 1); // Remove the last newline character
        return result;
    }
    return "";
}

size_t YamlNode::size() const {
    if (m_data_type == YamlNodeDataType::MAP) {
        return m_map.size();
    } else if (m_data_type == YamlNodeDataType::LIST) {
        return m_list.size();
    }
    throw std::runtime_error("Node is not iterable");
}

bool YamlNode::has_key(const std::string &key) const {
    if (m_data_type != YamlNodeDataType::MAP) {
        throw std::runtime_error("Node is not a dictionary");
    }
    return m_map.find(key) != m_map.end();
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
            if (indentation_before_hypen == -1) {
                indentation_before_hypen = indentation_length;
                ++indentation_length;
            }
            else {
                break;
            }
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

bool YamlNode::remove_matching_list_brackets(std::string *input_string, bool throw_on_mismatch, const std::vector<std::pair<char, char>> &matching_brackets) {
    for (const auto &bracket_pair : matching_brackets) {
        if (input_string->size() >= 2 && (*input_string)[0] == bracket_pair.first) {
            if ((*input_string)[input_string->size() - 1] == bracket_pair.second) {
                *input_string = input_string->substr(1, input_string->size() - 2);
                return true;
            }
            else {
                if (throw_on_mismatch) {
                    throw std::runtime_error("Mismatched surrounding list brackets in string: " + *input_string);
                }
            }
        }
    }
    return false;
}

std::pair<std::string,std::string> YamlNode::split_key_value(const std::string &line) {
    int n_double_quotes(0), n_single_quotes(0);
    bool previous_was_slash = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (!previous_was_slash && line[i] == '"') ++n_double_quotes;
        if (!previous_was_slash && line[i] == '\'') ++n_single_quotes;
        if (n_double_quotes % 2 == 0 && n_single_quotes % 2 == 0 && line[i] == ':') {
            string key = line.substr(0, i);
            strip_string_and_remove_quotation_marks(&key);
            string value = line.substr(i + 1);
            strip_string(&value);
            return {key, value};
        }
        previous_was_slash = (line[i] == '\\' && !previous_was_slash);
    }
    return {"", ""};
};

bool YamlNode::is_multiline_block() const {
    return m_data_type == YamlNodeDataType::MAP || m_data_type == YamlNodeDataType::LIST;
};

std::vector<std::string> YamlNode::get_elements_of_nested_structure(const std::string &input_string) {
    std::vector<std::string> elements;
    std::string current_element;
    std::vector<char> previous_separators;

    // get first and last non-whitespace characters of the input string
    size_t first_non_whitespace = input_string.find_first_not_of(" \t");
    size_t last_non_whitespace = input_string.find_last_not_of(" \t");
    if (first_non_whitespace == std::string::npos || last_non_whitespace == std::string::npos) {
        return {};
    }

    string_view trimmed_input(input_string.c_str() + first_non_whitespace, last_non_whitespace - first_non_whitespace + 1);

    bool previous_char_was_slash = false;
    for (char c : trimmed_input) {

        // if we are reading string inside quotes
        if (!previous_separators.empty() && (previous_separators.back() == '"' || previous_separators.back() == '\'')) {
            if (c == previous_separators.back() && !previous_char_was_slash) {
                previous_separators.pop_back();
            }
            else if (c == '/') {
                previous_char_was_slash = !previous_char_was_slash;
            }
            else {
                current_element += c;
            }
            continue;
        }

        if (c == ' ' || c == '\t') {
            continue;
        }

        // we care only about the outermost level for commas
        if (c == ',' && !current_element.empty() && previous_separators.empty()) {
            elements.push_back(current_element);
            current_element.clear();
            continue;
        }

        if (c == '{' || c == '[') {
            previous_separators.push_back(c);
        }

        if (c == '}' || c == ']') {
            if (previous_separators.empty() || (c == '}' && previous_separators.back() != '{') || (c == ']' && previous_separators.back() != '[')) {
                throw std::runtime_error("Unexpected closing character in nested structure");
            }
            previous_separators.pop_back();
        }
        current_element += c;
    }

    if (!current_element.empty()) {
        elements.push_back(current_element);
    }
    return elements;
}