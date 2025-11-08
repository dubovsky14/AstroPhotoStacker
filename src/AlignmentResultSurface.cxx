#include "../headers/AlignmentResultSurface.h"

#include "../headers/Common.h"

#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentResultSurface::AlignmentResultSurface() :
    AlignmentResultBase() {
        m_local_shifts_handler = make_unique<LocalShiftsHandler>();
};

AlignmentResultSurface::AlignmentResultSurface(const string &description_string) :
    AlignmentResultBase() {
    // parse the description string to extract the parameters
    vector<string> tokens = split_string(description_string, c_separator_in_description);
    if (tokens.size() < 2) {
        throw runtime_error("Invalid description string for AlignmentResultSurface: " + description_string);
    }
    m_ranking_score = stof(tokens[0]);
    const std::string local_shifts_string = tokens[1];
    m_local_shifts_handler = make_unique<LocalShiftsHandler>(local_shifts_string);
    m_is_valid = true;
};

AlignmentResultSurface::AlignmentResultSurface( const std::vector<LocalShift> &local_shifts, float ranking_score) :
    AlignmentResultBase() {
    set_parameters(local_shifts);
    m_ranking_score = ranking_score;
};


AlignmentResultSurface::AlignmentResultSurface(const AlignmentResultSurface &other) :
    AlignmentResultBase(other),
    m_local_shifts_handler(make_unique<LocalShiftsHandler>(*other.m_local_shifts_handler)) {
};

void AlignmentResultSurface::transform_from_reference_to_shifted_frame(float *x, float *y) const {
    m_local_shifts_handler->calculate_shifted_coordinates(x, y);
};

void AlignmentResultSurface::transform_to_reference_frame(float *x, float *y) const {
    float x_original = *x;
    float y_original = *y;
    m_local_shifts_handler->calculate_shifted_coordinates(&x_original, &y_original);
    const float dx = *x - x_original;
    const float dy = *y - y_original;
    *x -= dx;
    *y -= dy;
};

string AlignmentResultSurface::get_method_specific_description_string() const {
    return  to_string(m_ranking_score) + c_separator_in_description +
            m_local_shifts_handler->to_string();
};

void AlignmentResultSurface::set_parameters(const std::vector<LocalShift> &local_shifts) {
    m_local_shifts_handler = make_unique<LocalShiftsHandler>(local_shifts);
    m_is_valid = true;
};

void AlignmentResultSurface::draw_on_image(std::vector<std::vector<PixelType>> *image_data, int width, int height, bool image_in_reference_frame) const {
    if (m_local_shifts_handler == nullptr) {
        return;
    }

    const vector<LocalShift>& shifts = m_local_shifts_handler->get_shifts();
    const float radius = 3;

    for (const auto& shift : shifts) {
        float x = shift.x;
        float y = shift.y;

        if (image_in_reference_frame) {
            this->transform_to_reference_frame(&x, &y);
        }

        if (x < 0 || x >= width || y < 0 || y >= height) {
            continue;
        }

        draw_filled_circle_on_image(image_data, width, height, x, y, radius, std::vector<int>{255, 0, 0});
    }
};