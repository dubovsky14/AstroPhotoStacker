#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>

namespace AstroPhotoStacker {

    float random_uniform();

    template <class X>
    inline X pow2(X x) {return x*x;};

    bool contains(const std::string &main_string, const std::string &substring);

    bool starts_with(const std::string &main_string, const std::string &prefix);

    bool ends_with(const std::string &main_string, const std::string &suffix);

    bool string_is_int(const std::string &x);

    bool string_is_float(const std::string &x);

    std::vector<std::string> split_and_strip_string(std::string input_string, const std::string &separator);

    std::vector<std::string> split_string(std::string input_string, const std::string &separator);

    std::vector<std::string> split_by_whitespaces(std::string line);

    void strip_string(std::string *input_string, const std::string &chars_to_remove = " \n\t\r");

    void         to_upper(std::string *input_string);

    std::string  to_upper_copy(const std::string &input_string);

    // convert both strings to uppercase and compare
    bool compare_case_insensitive(const std::string &x, const std::string &y);

    template <class ResultType>
    ResultType convert_string_to(const std::string &input_string) {
        throw std::runtime_error ("Requested type not implemented!");
    };

    template <> inline
    int convert_string_to(const std::string &input_string)    {
        return std::stoi(input_string);
    };

    template <> inline
    unsigned int convert_string_to(const std::string &input_string)    {
        return std::stoul(input_string);
    };

    template <> inline
    unsigned long long int convert_string_to(const std::string &input_string)    {
        return std::stoull(input_string);
    };

    template <> inline
    float convert_string_to(const std::string &input_string)    {
        return std::stod(input_string);
    };

    template <> inline
    double convert_string_to(const std::string &input_string)    {
        return std::stod(input_string);
    };

    template <> inline
    std::string convert_string_to(const std::string &input_string)    {
        return input_string;
    };

    template <> inline
    bool convert_string_to(const std::string &input_string)    {
        const std::string input_upper = AstroPhotoStacker::to_upper_copy(input_string);
        if      (input_upper == "TRUE")  return true;
        else if (input_upper == "FALSE") return false;
        else {
            throw std::runtime_error("String \"" + input_string + "\" can't be converted to bool value!");
        }
    };

    template <> inline
    std::vector<int> convert_string_to(const std::string &input_string)    {
        std::string stripped_string = input_string;
        strip_string(&stripped_string, "([{}})");
        std::vector<std::string> elements = split_and_strip_string(stripped_string, ",");

        std::vector<int> result;
        for (const std::string &element : elements) {
            result.push_back(convert_string_to<int>(element));
        }
        return result;
    };

    template <> inline
    std::vector<unsigned long long int> convert_string_to(const std::string &input_string)    {
        std::string stripped_string = input_string;
        strip_string(&stripped_string, "([{}})");
        std::vector<std::string> elements = split_and_strip_string(stripped_string, ",");

        std::vector<unsigned long long int> result;
        for (const std::string &element : elements) {
            result.push_back(convert_string_to<unsigned long long int>(element));
        }
        return result;
    };

    std::vector<std::string> get_raw_files_in_folder(const std::string &folder_address);

    std::string round_and_convert_to_string(double x, int digits_after_decimal_point = 1);

    template <class T>
    T force_range(T x, T min_value, T max_value) {
        return std::max(min_value, std::min(max_value, x));
    }
}