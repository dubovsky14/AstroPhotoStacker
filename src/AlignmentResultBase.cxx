#include "../headers/AlignmentResultBase.h"

#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;


std::string AlignmentResultBase::get_description_string() const  {
    return get_type_name() + s_type_separator + get_method_specific_description_string() + s_type_separator + m_frame_score.to_string();
};

std::array<std::string, 3> AlignmentResultBase::split_type_and_description_and_score(const std::string &description_string) {
    size_t separator_pos_first = description_string.find(s_type_separator);
    size_t separator_pos_last = description_string.rfind(s_type_separator);
    if (separator_pos_first == std::string::npos) {
        throw runtime_error("Invalid description string: " + description_string);
    }
    size_t end_of_method_specific_string = (separator_pos_first == separator_pos_last) ? std::string::npos : separator_pos_last;
    const std::string type_string = description_string.substr(0, separator_pos_first);
    const std::string method_specific_string = description_string.substr(separator_pos_first + s_type_separator.length(), end_of_method_specific_string);
    const std::string score_string = (separator_pos_first == separator_pos_last) ? "" : description_string.substr(separator_pos_last + s_type_separator.length());

    return {type_string, method_specific_string, score_string};
};