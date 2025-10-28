#include "../headers/AlignmentResultBase.h"

#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;


std::string AlignmentResultBase::get_description_string(const std::string &type_name) const  {
    return type_name + s_type_separator + get_method_specific_description_string();
};

std::pair<std::string, std::string> AlignmentResultBase::split_type_and_description(const std::string &description_string) {
    size_t separator_pos = description_string.find(s_type_separator);
    if (separator_pos == std::string::npos) {
        throw runtime_error("Invalid description string: " + description_string);
    }
    std::string type_string = description_string.substr(0, separator_pos);
    std::string rest_of_description = description_string.substr(separator_pos + s_type_separator.length());
    return {type_string, rest_of_description};
};