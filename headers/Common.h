#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>
#include <map>

#include "../headers/PixelType.h"

namespace AstroPhotoStacker {

    /**
     * @brief Generate a random number between 0 and 1
     *
     * @return float The generated random number
    */
    float random_uniform();

    /**
     * @brief Generate a random number between a and b
     *
     * @return float The generated random number
    */
    float random_uniform(float a, float b);

    /**
     * @brief Calculate 2nd root of a number
     *
     * @tparam X The type of the number
     * @param x The number to calculate the 2nd root of
     * @return X The 2nd root of the number
    */
    template <class X>
    inline X pow2(X x) {return x*x;};

    /**
     * @brief Check if the main_string contains the substring
     *
     * @param main_string The main string to search in
     * @param substring The substring to search for
     * @return bool True if the main_string contains the substring, false otherwise
    */
    bool contains(const std::string &main_string, const std::string &substring);

    /**
     * @brief Check if the main_string starts with the prefix
     *
     * @param main_string The main string to check
     * @param prefix The prefix to check for
     * @return bool True if the main_string starts with the prefix, false otherwise
    */
    bool starts_with(const std::string &main_string, const std::string &prefix);

    /**
     * @brief Check if the main_string ends with the suffix
     *
     * @param main_string The main string to check
     * @param suffix The suffix to check for
     * @return bool True if the main_string ends with the suffix, false otherwise
    */
    bool ends_with(const std::string &main_string, const std::string &suffix);

    /**
     * @brief Check if the string represents an integer
     *
     * @param x The string to check
     * @return bool True if the string represents an integer, false otherwise
    */
    bool string_is_int(const std::string &x);

    /**
     * @brief Check if the string represents a floating-point number
     *
     * @param x The string to check
     * @return bool True if the string represents a floating-point number, false otherwise
    */
    bool string_is_float(const std::string &x);

    /**
     * @brief Split and strip a string based on a separator
     *
     * @param input_string The input string to split and strip
     * @param separator The separator to split the string by
     * @return std::vector<std::string> The vector of split and stripped strings
    */
    std::vector<std::string> split_and_strip_string(std::string input_string, const std::string &separator);

    /**
     * @brief Split a string based on a separator
     *
     * @param input_string The input string to split
     * @param separator The separator to split the string by
     * @return std::vector<std::string> The vector of split strings
    */
    std::vector<std::string> split_string(std::string input_string, const std::string &separator);

    /**
     * @brief Split a string by whitespaces
     *
     * @param line The input string to split
     * @return std::vector<std::string> The vector of split strings
    */
    std::vector<std::string> split_by_whitespaces(std::string line);

    /**
     * @brief Strip characters from a string
     *
     * @param input_string The input string to strip
     * @param chars_to_remove The characters to remove from the string - default are whitespaces
    */
    void strip_string(std::string *input_string, const std::string &chars_to_remove = " \n\t\r");

    /**
     * @brief Convert a string to uppercase in-place
     *
     * @param input_string The input string to convert to uppercase
    */
    void to_upper(std::string *input_string);

    /**
     * @brief Convert a string to uppercase and return a copy
     *
     * @param input_string The input string to convert to uppercase
     * @return std::string The uppercase string
    */
    std::string to_upper_copy(const std::string &input_string);

    /**
     * @brief Compare two strings in a case-insensitive manner
     *
     * @param x The first string to compare
     * @param y The second string to compare
     * @return bool True if the strings are equal (ignoring case), false otherwise
    */
    bool compare_case_insensitive(const std::string &x, const std::string &y);

    /**
     * @brief Convert a string to the specified type
     *
     * @tparam ResultType The type to convert the string to
     * @param input_string The input string to convert
     * @return ResultType The converted value
    */
    template <class ResultType>
    ResultType convert_string_to(const std::string &input_string) {
        throw std::runtime_error ("Requested type not implemented!");
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to an integer
     *
     * @param input_string The input string to convert
     * @return int The converted integer value
    */
    template <> inline
    int convert_string_to(const std::string &input_string)    {
        return std::stoi(input_string);
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to an unsigned integer
     *
     * @param input_string The input string to convert
     * @return unsigned int The converted unsigned integer value
    */
    template <> inline
    unsigned int convert_string_to(const std::string &input_string)    {
        return std::stoul(input_string);
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to an unsigned long long integer
     *
     * @param input_string The input string to convert
     * @return unsigned long long int The converted unsigned long long integer value
    */
    template <> inline
    unsigned long long int convert_string_to(const std::string &input_string)    {
        return std::stoull(input_string);
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to a float
     *
     * @param input_string The input string to convert
     * @return float The converted float value
    */
    template <> inline
    float convert_string_to(const std::string &input_string)    {
        return std::stod(input_string);
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to a double
     *
     * @param input_string The input string to convert
     * @return double The converted double value
    */
    template <> inline
    double convert_string_to(const std::string &input_string)    {
        return std::stod(input_string);
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to a string (no conversion needed)
     *
     * @param input_string The input string to convert
     * @return std::string The input string itself
    */
    template <> inline
    std::string convert_string_to(const std::string &input_string)    {
        return input_string;
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to a boolean
     *
     * @param input_string The input string to convert
     * @return bool The converted boolean value
    */
    template <> inline
    bool convert_string_to(const std::string &input_string)    {
        const std::string input_upper = AstroPhotoStacker::to_upper_copy(input_string);
        if      (input_upper == "TRUE")  return true;
        else if (input_upper == "FALSE") return false;
        else {
            throw std::runtime_error("String \"" + input_string + "\" can't be converted to bool value!");
        }
    };

    /**
     * @brief Specialization of convert_string_to for converting a string to a vector of integers
     *
     * @param input_string The input string to convert
     * @return std::vector<int> The converted vector of integers
    */
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

    /**
     * @brief Specialization of convert_string_to for converting a string to a vector of unsigned long long integers
     *
     * @param input_string The input string to convert
     * @return std::vector<unsigned long long int> The converted vector of unsigned long long integers
    */
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

    /**
     * @brief equivalent to python's join function
     *
     * @param std::string
     * @param std::vector<std::string>
     * @return std::string
     */
    std::string join_strings(const std::string &separator, const std::vector<std::string> &strings);

    /**
    * @brief Find the n-th occurrence of a substring in a string
    *
    * @param main_string The main string to search in
    * @param substring The substring to search for
    * @param n The occurrence number to find (1-based index)
    * @return int The index of the n-th occurrence of the substring, or -1 if not found
    */
    int find_nth_occurrence(const std::string &main_string, const std::string &substring, int n);

    /**
     * @brief Get the list of raw files in a folder
     *
     * @param folder_address The address of the folder
     * @return std::vector<std::string> The vector of raw file names in the folder
    */
    std::vector<std::string> get_frame_files_in_folder(const std::string &folder_address);

    /**
     * @brief Round a number and convert it to a string
     *
     * @param x The number to round and convert
     * @param digits_after_decimal_point The number of digits after the decimal point in the resulting string
     * @return std::string - string representation of the rounded number
    */
    std::string round_and_convert_to_string(double x, int digits_after_decimal_point = 1);

    /**
     * @brief Force a value to be within a specified range
     *
     * @tparam T The type of the value
     * @param x The value to force within the range
     * @param min_value The minimum value of the range
     * @param max_value The maximum value of the range
     * @return T The value forced within the range
    */
    template <class T>
    T force_range(T x, T min_value, T max_value) {
        return std::max(min_value, std::min(max_value, x));
    }

    template <class KeyType, class ValueType>
    ValueType get_with_default(const std::map<KeyType, ValueType> &map, const KeyType &key, const ValueType &default_value) {
        auto it = map.find(key);
        if (it == map.end()) {
            return default_value;
        }
        return it->second;
    }

    template <class ElementType>
    std::vector<std::vector<ElementType>> get_transponded_vector(const std::vector<std::vector<ElementType>> &input) {
        if (input.size() == 0) {
            return std::vector<std::vector<ElementType>>();
        }
        const unsigned int num_rows = input.size();
        const unsigned int num_columns = input[0].size();
        std::vector<std::vector<ElementType>> result(num_columns, std::vector<ElementType>(num_rows));
        for (unsigned int i_row = 0; i_row < num_rows; i_row++) {
            for (unsigned int i_column = 0; i_column < num_columns; i_column++) {
                result[i_column][i_row] = input[i_row][i_column];
            }
        }
        return result;
    }

    std::string unix_time_to_string(int unix_time);

    template <typename PixelValueTypeInput, typename PixelValueTypeOutput = PixelValueTypeInput>
    std::vector<PixelValueTypeOutput> convert_color_to_monochrome(const std::vector<std::vector<PixelValueTypeInput>> &color_image, int width, int height) {
        const unsigned int n_pixels = width * height;
        const unsigned int n_colors = color_image.size();

        std::vector<float> temp_result(n_pixels,0);
        for (unsigned int i_color = 0; i_color < n_colors; i_color++) {
            const std::vector<PixelValueTypeInput> &color_channel = color_image[i_color];
            for (unsigned int i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
                temp_result[i_pixel] += color_channel[i_pixel];
            }
        }

        std::vector<PixelValueTypeOutput> result(n_pixels);
        for (unsigned int i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
            result[i_pixel] = static_cast<PixelValueTypeOutput>(temp_result[i_pixel] / n_colors);
        }
        return result;
    }

    std::string get_filename_from_path(const std::string &path);

    std::string get_filename_without_extension(const std::string &path);

    // python-style string repetition
    std::string operator*(int n_repeats, const std::string& str);

    // python-style string repetition
    std::string operator*(const std::string& str, int n_repeats);

    std::vector<std::string> get_formated_table(const std::vector<std::vector<std::string>> &data, const std::string &separator);

    void draw_filled_circle_on_image(std::vector<std::vector<PixelType>> *image_data, int width, int height, int center_x, int center_y, int radius, const std::vector<int> &color);

    std::string replace_file_extension(const std::string &file_address, const std::string &new_extension);

    template<typename InptutPixelType>
    std::vector<std::vector<unsigned short>> scale_image_to_16bit_int(const std::vector<std::vector<InptutPixelType>> &input_image) {
        std::vector<std::vector<unsigned short>> result;
        InptutPixelType max_value = 0;
        for (const std::vector<InptutPixelType> &input_channel : input_image) {
            for (InptutPixelType value : input_channel) {
                if (value > max_value) {
                    max_value = value;
                }
            }
            result.push_back(std::vector<unsigned short>(input_channel.size()));
        }
        const double scale_factor = 65535.0f / max_value;

        for (size_t i_channel = 0; i_channel < input_image.size(); i_channel++) {
            const std::vector<InptutPixelType> &input_channel = input_image[i_channel];
            std::vector<unsigned short> &output_channel = result[i_channel];
            for (size_t i = 0; i < input_channel.size(); i++) {
                output_channel[i] = static_cast<unsigned short>(input_channel[i] * scale_factor);
            }
        }
        return result;
    };
}

