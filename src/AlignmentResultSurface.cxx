#include "../headers/AlignmentResultSurface.h"

#include "../headers/Common.h"

#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentResultSurface::AlignmentResultSurface() :
    AlignmentResultBase(),
    m_geometric_transformer(make_unique<GeometricTransformer>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)) {
};

AlignmentResultSurface::AlignmentResultSurface(const string &description_string) :
    AlignmentResultBase() {
    // parse the description string to extract the parameters
    vector<string> tokens = split_string(description_string, c_separator_in_description);
    if (tokens.size() < 7) {
        throw runtime_error("Invalid description string for AlignmentResultSurface: " + description_string);
    }
    float shift_x = stof(tokens[0]);
    float shift_y = stof(tokens[1]);
    float rotation_center_x = stof(tokens[2]);
    float rotation_center_y = stof(tokens[3]);
    float rotation = stof(tokens[4]);
    m_ranking_score = stof(tokens[5]);
    const std::string local_shifts_string = tokens[6];
    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x,
                                                                shift_y,
                                                                rotation_center_x,
                                                                rotation_center_y,
                                                                rotation);

    m_local_shifts_handler = make_unique<LocalShiftsHandler>(local_shifts_string);
    m_is_valid = true;
};

AlignmentResultSurface::AlignmentResultSurface( float shift_x,
                                                float shift_y,
                                                float rotation_center_x,
                                                float rotation_center_y,
                                                float rotation,
                                                const std::vector<LocalShift> &local_shifts,
                                                float ranking_score) :
    AlignmentResultBase() {
    set_parameters(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation, local_shifts);
    m_ranking_score = ranking_score;
};


AlignmentResultSurface::AlignmentResultSurface(const AlignmentResultSurface &other) :
    AlignmentResultBase(other),
    m_geometric_transformer(make_unique<GeometricTransformer>(*other.m_geometric_transformer)),
    m_local_shifts_handler(make_unique<LocalShiftsHandler>(*other.m_local_shifts_handler)) {
};

void AlignmentResultSurface::transform_from_reference_to_shifted_frame(float *x, float *y) const {
    m_geometric_transformer->transform_from_reference_to_shifted_frame(x, y);
    m_local_shifts_handler->calculate_shifted_coordinates(x, y);
};

void AlignmentResultSurface::transform_to_reference_frame(float *x, float *y) const {
    m_geometric_transformer->transform_to_reference_frame(x, y);
    float x_original = *x;
    float y_original = *y;
    m_local_shifts_handler->calculate_shifted_coordinates(&x_original, &y_original);
    const float dx = *x - x_original;
    const float dy = *y - y_original;
    *x -= dx;
    *y -= dy;
};

string AlignmentResultSurface::get_method_specific_description_string() const {
    float shift_x, shift_y, rotation_center_x, rotation_center_y, rotation;
    m_geometric_transformer->get_parameters(&shift_x, &shift_y, &rotation_center_x, &rotation_center_y, &rotation);

    return  to_string(shift_x) + c_separator_in_description +
            to_string(shift_y) + c_separator_in_description +
            to_string(rotation_center_x) + c_separator_in_description +
            to_string(rotation_center_y) + c_separator_in_description +
            to_string(rotation) + c_separator_in_description +
            to_string(m_ranking_score) + c_separator_in_description +
            m_local_shifts_handler->to_string();
};

void AlignmentResultSurface::set_parameters(float shift_x,
                                            float shift_y,
                                            float rotation_center_x,
                                            float rotation_center_y,
                                            float rotation,
                                            const std::vector<LocalShift> &local_shifts) {
    m_geometric_transformer = make_unique<GeometricTransformer>(
        shift_x,
        shift_y,
        rotation_center_x,
        rotation_center_y,
        rotation
    );
    m_local_shifts_handler = make_unique<LocalShiftsHandler>(local_shifts);
    m_is_valid = true;
};