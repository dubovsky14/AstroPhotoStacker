#include "../headers/AlignmentResultTranslationOnly.h"

#include "../headers/Common.h"

#include <stdexcept>
using namespace std;
using namespace AstroPhotoStacker;

AlignmentResultTranslationOnly::AlignmentResultTranslationOnly() :
    AlignmentResultBase() {
};

AlignmentResultTranslationOnly::AlignmentResultTranslationOnly(const string &description_string) :
    AlignmentResultBase() {
    // parse the description string to extract the parameters
    vector<string> tokens = split_string(description_string, c_separator_in_description);
    if (tokens.size() < 3) {
        throw runtime_error("Invalid description string for AlignmentResultTranslationOnly: " + description_string);
    }
    m_shift_x = stof(tokens[0]);
    m_shift_y = stof(tokens[1]);
    m_ranking_score = stof(tokens[2]);
    m_is_valid = true;
};

AlignmentResultTranslationOnly::AlignmentResultTranslationOnly( float shift_x,
                                                                float shift_y) :
    AlignmentResultBase(),
    m_shift_x(shift_x),
    m_shift_y(shift_y) {
        m_is_valid = true;
};

AlignmentResultTranslationOnly::AlignmentResultTranslationOnly(const AlignmentResultTranslationOnly &other) :
    AlignmentResultBase(other),
    m_shift_x(other.m_shift_x),
    m_shift_y(other.m_shift_y) {
};

void AlignmentResultTranslationOnly::transform_from_reference_to_shifted_frame(float *x, float *y) const {
    *x -= m_shift_x;
    *y -= m_shift_y;
};

void AlignmentResultTranslationOnly::transform_to_reference_frame(float *x, float *y) const {
    *x += m_shift_x;
    *y += m_shift_y;
};

std::string AlignmentResultTranslationOnly::get_method_specific_description_string() const {
    return to_string(m_shift_x) + c_separator_in_description +
           to_string(m_shift_y) + c_separator_in_description +
           to_string(m_ranking_score);
};

void AlignmentResultTranslationOnly::get_shift(float *shift_x, float *shift_y) const {
    if (shift_x) *shift_x = m_shift_x;
    if (shift_y) *shift_y = m_shift_y;
};

void AlignmentResultTranslationOnly::set_shift(float shift_x, float shift_y) {
    m_shift_x = shift_x;
    m_shift_y = shift_y;
};
