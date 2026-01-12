#include "../headers/AlignmentResultPlateSolving.h"

#include "../headers/Common.h"

#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;


AlignmentResultPlateSolving::AlignmentResultPlateSolving() :
    AlignmentResultBase(),
    m_geometric_transformer(make_unique<GeometricTransformer>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)) {
};

AlignmentResultPlateSolving::AlignmentResultPlateSolving(const string &description_string) :
    AlignmentResultBase() {
    // parse the description string to extract the parameters
    vector<string> tokens = split_string(description_string, c_separator_in_description);
    if (tokens.size() < 6) {
        throw runtime_error("Invalid description string for AlignmentResultPlateSolving: " + description_string);
    }
    float shift_x = stof(tokens[0]);
    float shift_y = stof(tokens[1]);
    float rotation_center_x = stof(tokens[2]);
    float rotation_center_y = stof(tokens[3]);
    float rotation = stof(tokens[4]);
    float zoom = 1.0f;
    if (tokens.size() == 6) {   // yeah, this is a mess, but we need to keep compatibility with older versions
        m_ranking_score = stof(tokens[5]);
    } else if (tokens.size() == 7) {
        m_ranking_score = stof(tokens[6]);
        zoom = stof(tokens[5]);
    }

    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x,
                                                                shift_y,
                                                                rotation_center_x,
                                                                rotation_center_y,
                                                                rotation,
                                                                zoom);
    m_is_valid = true;
};

AlignmentResultPlateSolving::AlignmentResultPlateSolving(   float shift_x,
                                                            float shift_y,
                                                            float rotation_center_x,
                                                            float rotation_center_y,
                                                            float rotation,
                                                            float zoom) :
    AlignmentResultBase() {
    set_parameters(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation, zoom);
};


AlignmentResultPlateSolving::AlignmentResultPlateSolving(const AlignmentResultPlateSolving &other) :
    AlignmentResultBase(other),
    m_geometric_transformer(make_unique<GeometricTransformer>(*other.m_geometric_transformer))  {
};

AlignmentResultPlateSolving& AlignmentResultPlateSolving::operator=(const AlignmentResultPlateSolving &other) {
    if (this != &other) {
        copy_base_data(other);
        m_geometric_transformer = std::make_unique<GeometricTransformer>(*other.m_geometric_transformer);
    }
    return *this;
};


void AlignmentResultPlateSolving::transform_from_reference_to_shifted_frame(float *x, float *y) const {
    m_geometric_transformer->transform_from_reference_to_shifted_frame(x, y);
};

void AlignmentResultPlateSolving::transform_to_reference_frame(float *x, float *y) const {
    m_geometric_transformer->transform_to_reference_frame(x, y);
};

string AlignmentResultPlateSolving::get_method_specific_description_string() const {
    float shift_x, shift_y, rotation_center_x, rotation_center_y, rotation, zoom;
    m_geometric_transformer->get_parameters(&shift_x, &shift_y, &rotation_center_x, &rotation_center_y, &rotation, &zoom);

    return  to_string(shift_x) + c_separator_in_description +
            to_string(shift_y) + c_separator_in_description +
            to_string(rotation_center_x) + c_separator_in_description +
            to_string(rotation_center_y) + c_separator_in_description +
            to_string(rotation) + c_separator_in_description +
            to_string(zoom) + c_separator_in_description +
            to_string(m_ranking_score);
};

void AlignmentResultPlateSolving::get_parameters(float *shift_x,
                    float *shift_y,
                    float *rotation_center_x,
                    float *rotation_center_y,
                    float *rotation,
                    float *zoom) const {
    m_geometric_transformer->get_parameters(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation, zoom);
};

void AlignmentResultPlateSolving::set_parameters(   float shift_x,
                                                    float shift_y,
                                                    float rotation_center_x,
                                                    float rotation_center_y,
                                                    float rotation,
                                                    float zoom) {
    m_geometric_transformer = make_unique<GeometricTransformer>(
        shift_x,
        shift_y,
        rotation_center_x,
        rotation_center_y,
        rotation,
        zoom
    );
    m_is_valid = true;
};