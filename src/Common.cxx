#include "../headers/Common.h"


#include <string>
#include <cmath>
#include <algorithm>
#include <filesystem>

using namespace std;

float AstroPhotoStacker::random_uniform()  {
    // Be carefull about the brackets, otherwise RAND_MAX will overflow
    return (float(rand())) / (float(RAND_MAX)+1);
}

float AstroPhotoStacker::random_uniform(float a, float b)  {
    return a + (b-a)*random_uniform();
};

bool AstroPhotoStacker::contains(const string &main_string, const string &substring)    {
    return (main_string.find(substring)) != std::string::npos;
};

bool AstroPhotoStacker::starts_with(const string &main_string, const string &prefix)    {
    const unsigned int prefix_lenght = prefix.length();
    const unsigned int string_lenght = main_string.length();
    if (prefix_lenght > string_lenght) return false;
    return prefix == main_string.substr(0, prefix_lenght);
};

bool AstroPhotoStacker::ends_with(const string &main_string, const string &suffix)    {
    const unsigned int suffix_lenght = suffix.length();
    const unsigned int string_lenght = main_string.length();
    if (suffix_lenght > string_lenght) return false;
    return suffix == main_string.substr(string_lenght-suffix_lenght, string_lenght);
};


bool AstroPhotoStacker::string_is_int(const string &x)    {
    try {
        std::stoi(x);
    }
    catch (...) {
        return false;
    }
    return true;
};

bool AstroPhotoStacker::string_is_float(const string &x)   {
    try {
        std::stod(x);
    }
    catch (...) {
        return false;
    }
    return true;
};

vector<string> AstroPhotoStacker::split_and_strip_string(string input_string, const string &separator) {
    vector<string> result = split_string(input_string, separator);
    for (string &x : result)    {
        strip_string(&x, " \n\t\r");
    }
    return result;
}

vector<string> AstroPhotoStacker::split_string(string input_string, const string &separator)    {
    vector<string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = input_string.find(separator)) != std::string::npos) {
        result.push_back(input_string.substr(0, pos));
        input_string.erase(0, pos + separator.length());
    }
    if (input_string.length() > 0) result.push_back(input_string);
    return result;
};

std::vector<std::string> AstroPhotoStacker::split_by_whitespaces(std::string line)   {
    const string white_space_chars = " \t\r\n";
    strip_string(&line, white_space_chars);
    std::vector<std::string> result;
    while (line.size() > 0) {
        const std::size_t first_white_space = line.find_first_of(white_space_chars);
        if (first_white_space == std::string::npos) {
            result.push_back(line);
            break;
        }
        else    {
            result.push_back(line.substr(0, first_white_space));
            line.erase(0, first_white_space);
            strip_string(&line, white_space_chars);
        }
    }
    return result;
};

void AstroPhotoStacker::strip_string(string *input_string, const string &chars_to_remove)    {
    input_string->erase(0,input_string->find_first_not_of(chars_to_remove));
    input_string->erase(input_string->find_last_not_of(chars_to_remove)+1);
}

void AstroPhotoStacker::to_upper(std::string *input_string) {
    std::transform(input_string->begin(), input_string->end(),input_string->begin(), ::toupper);
};

std::string  AstroPhotoStacker::to_upper_copy(const std::string &input_string)  {
    string result = input_string;
    AstroPhotoStacker::to_upper(&result);
    return result;
};

// convert both strings to uppercase and compare
bool AstroPhotoStacker::compare_case_insensitive(const std::string &x, const std::string &y)    {
    const string x_upper = AstroPhotoStacker::to_upper_copy(x);
    const string y_upper = AstroPhotoStacker::to_upper_copy(y);
    return x_upper == y_upper;
};

std::string AstroPhotoStacker::join_strings(const std::string &separator, const std::vector<std::string> &strings)    {
    std::string result;
    for (const string &x : strings)   {
        result += x + separator;
    }
    if (result.size() > 0) result.erase(result.size()-separator.size());
    return result;
};

int AstroPhotoStacker::find_nth_occurrence(const std::string &main_string, const std::string &substring, int n) {
    size_t pos = 0;
    int count = 0;

    while (count < n) {
        pos = main_string.find(substring, pos);
        if (pos == std::string::npos) {
            return -1; // Not found
        }
        count++;
        pos += substring.length(); // Move past the last found occurrence
    }

    return static_cast<int>(pos - substring.length()); // Return the position of the n-th occurrence
}

std::vector<std::string> AstroPhotoStacker::get_frame_files_in_folder(const std::string &folder_address)  {
    vector<string> result;
    for (const auto &entry : filesystem::directory_iterator(folder_address)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        if (filesystem::is_directory(entry)) {
            continue;
        }
        result.push_back(entry.path());
    }
    sort(result.begin(), result.end());
    return result;
};

std::string AstroPhotoStacker::round_and_convert_to_string(double x, int digits_after_decimal_point) {
    const double factor = pow(10, digits_after_decimal_point);
    const long int y = round(factor*x);

    const long int rounded_int = y / factor;
    const long int rounded_fraction = abs(y % int(factor));
    int zeros_to_add = digits_after_decimal_point - to_string(rounded_fraction).length();
    string string_rounded = to_string(rounded_int) + ".";
    for (int i = 0; i < zeros_to_add; i++) {
        string_rounded = string_rounded + "0";
    }
    string_rounded = string_rounded + to_string(rounded_fraction);
    return string_rounded;
};

std::string AstroPhotoStacker::unix_time_to_string(int unix_time)  {
    time_t t = unix_time;
    struct tm *tm = localtime(&t);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    return buffer;
};

std::string AstroPhotoStacker::get_filename_from_path(const std::string &path)  {
    std::filesystem::path p(path);
    return p.filename().string();
};

std::string AstroPhotoStacker::get_filename_without_extension(const std::string &path)  {
    std::filesystem::path p(path);
    return p.stem().string();
};

std::string AstroPhotoStacker::operator*(int n_repeats, const std::string& str) {
    std::string result;
    for (int i = 0; i < n_repeats; ++i) {
        result += str;
    }
    return result;
};

std::string AstroPhotoStacker::operator*(const std::string& str, int n_repeats)    {
    return n_repeats * str;
};

std::vector<std::string> AstroPhotoStacker::get_formated_table(const std::vector<std::vector<std::string>> &data, const std::string &separator) {
    if (data.size() == 0)   {
        return {};
    }
    vector<unsigned int> max_lengths(data[0].size());
    for (const auto &row : data) {
        if (row.size() != max_lengths.size()) {
            throw std::runtime_error("All rows must have the same number of columns");
        }
        for (size_t col = 0; col < row.size(); ++col) {
            max_lengths[col] = std::max<unsigned int>(max_lengths[col], row[col].length());
        }
    }

    std::vector<std::string> result;
    for (const auto &row : data) {
        std::vector<std::string> formatted_row;
        for (size_t col = 0; col < row.size(); ++col) {
            formatted_row.push_back(row[col] + (" "s * (max_lengths[col] - row[col].length())));
        }
        result.push_back(AstroPhotoStacker::join_strings(separator, formatted_row));
    }
    return result;
};

void AstroPhotoStacker::draw_filled_circle_on_image(std::vector<std::vector<PixelType>> *image_data, int width, int height, int center_x, int center_y, int radius, const std::vector<int> &color) {
    const int radius_squared = radius * radius;
    const int x_start = std::max(0, center_x - radius);
    const int x_end = std::min(width - 1, center_x + radius);
    const int y_start = std::max(0, center_y - radius);
    const int y_end = std::min(height - 1, center_y + radius);

    for (int y = y_start; y <= y_end; ++y) {
        for (int x = x_start; x <= x_end; ++x) {
            const int dx = x - center_x;
            const int dy = y - center_y;
            if (dx * dx + dy * dy <= radius_squared) {
                for (size_t c = 0; c < color.size(); ++c) {
                    (*image_data)[c][y * width + x] = static_cast<PixelType>(color[c]);
                }
            }
        }
    }
};