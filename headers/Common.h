#pragma once
#include <cmath>
#include <vector>
#include <string>

namespace AstroPhotoStacker {

    float RandomUniform();

    template <class X>
    inline X pow2(X x) {return x*x;};

    bool Contains(const std::string &main_string, const std::string &substring);

    bool StartsWith(const std::string &main_string, const std::string &prefix);

    bool EndsWith(const std::string &main_string, const std::string &suffix);


    bool StringIsInt(const std::string &x);

    bool StringIsFloat(const std::string &x);

    std::vector<std::string> SplitAndStripString(std::string input_string, const std::string &separator);

    std::vector<std::string> SplitString(std::string input_string, const std::string &separator);

    std::vector<std::string> SplitByWhitespaces(std::string line);

    void StripString(std::string *input_string, const std::string &chars_to_remove = " \n\t\r");

    void         to_upper(std::string *input_string);

    std::string  to_upper_copy(const std::string &input_string);

    // convert both strings to uppercase and compare
    bool CompareCaseInsensitive(const std::string &x, const std::string &y);

    template <class ResultType>
    ResultType ConvertStringTo(const std::string &input_string) {
        throw std::string ("Requested type not implemented!");
    };

    template <> inline
    int ConvertStringTo(const std::string &input_string)    {
        return std::stoi(input_string);
    };

    template <> inline
    unsigned long long int ConvertStringTo(const std::string &input_string)    {
        return std::stoull(input_string);
    };

    template <> inline
    float ConvertStringTo(const std::string &input_string)    {
        return std::stod(input_string);
    };

    template <> inline
    double ConvertStringTo(const std::string &input_string)    {
        return std::stod(input_string);
    };

    template <> inline
    std::string ConvertStringTo(const std::string &input_string)    {
        return input_string;
    };

    template <> inline
    bool ConvertStringTo(const std::string &input_string)    {
        const std::string input_upper = AstroPhotoStacker::to_upper_copy(input_string);
        if      (input_upper == "TRUE")  return true;
        else if (input_upper == "FALSE") return false;
        else {
            throw std::string("String \"" + input_string + "\" can't be converted to bool value!");
        }
    };

    template <> inline
    std::vector<int> ConvertStringTo(const std::string &input_string)    {
        std::string stripped_string = input_string;
        StripString(&stripped_string, "([{}})");
        std::vector<std::string> elements = SplitAndStripString(stripped_string, ",");

        std::vector<int> result;
        for (const std::string &element : elements) {
            result.push_back(ConvertStringTo<int>(element));
        }
        return result;
    };

    template <> inline
    std::vector<unsigned long long int> ConvertStringTo(const std::string &input_string)    {
        std::string stripped_string = input_string;
        StripString(&stripped_string, "([{}})");
        std::vector<std::string> elements = SplitAndStripString(stripped_string, ",");

        std::vector<unsigned long long int> result;
        for (const std::string &element : elements) {
            result.push_back(ConvertStringTo<unsigned long long int>(element));
        }
        return result;
    };
}