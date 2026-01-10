#include "../headers/LensCorrectionsTool.h"

#include <cmath>



using namespace AstroPhotoStacker;
using namespace std;


LensCorrectionsTool::LensCorrectionsTool(int image_width, int image_height) {
    m_image_width = image_width;
    m_image_height = image_height;
    m_center_x = image_width / 2.0;
    m_center_y = image_height / 2.0;
};

void LensCorrectionsTool::check_resolution_consistency(int image_width, int image_height) const {
    if (image_width != m_image_width || image_height != m_image_height) {
        throw std::runtime_error("LensCorrectionsTool: Image resolution does not match the initialized resolution.");
    }
};

void LensCorrectionsTool::set_radial_distortion_coefficients(double k1, double k2, double k3)    {
    set_radial_distortion_coefficient_k1(k1);
    set_radial_distortion_coefficient_k2(k2);
    set_radial_distortion_coefficient_k3(k3);
};

void LensCorrectionsTool::set_radial_distortion_coefficient_k1(double k1) {
    m_k1 = k1;
};

double LensCorrectionsTool::get_radial_distortion_coefficient_k1() const {
    return m_k1;
};

void LensCorrectionsTool::set_radial_distortion_coefficient_k2(double k2) {
    m_k2 = k2;
};

double LensCorrectionsTool::get_radial_distortion_coefficient_k2() const {
    return m_k2;
};

void LensCorrectionsTool::set_radial_distortion_coefficient_k3(double k3) {
    m_k3 = k3;
};

double LensCorrectionsTool::get_radial_distortion_coefficient_k3() const {
    return m_k3;
};

void LensCorrectionsTool::set_sensor_offset(double offset_in_pixels_x, double offset_in_pixels_y)    {
    set_sensor_offset_x(offset_in_pixels_x);
    set_sensor_offset_y(offset_in_pixels_y);
};

void LensCorrectionsTool::set_sensor_offset_x(double offset_in_pixels_x) {
    m_sensor_offset_x = offset_in_pixels_x;
    m_center_x = (m_image_width / 2.0) + m_sensor_offset_x;
};

void LensCorrectionsTool::set_sensor_offset_y(double offset_in_pixels_y) {
    m_sensor_offset_y = offset_in_pixels_y;
    m_center_y = (m_image_height / 2.0) + m_sensor_offset_y;
};

double LensCorrectionsTool::get_sensor_offset_x() const {
    return m_sensor_offset_x;
};

double LensCorrectionsTool::get_sensor_offset_y() const {
    return m_sensor_offset_y;
};

void LensCorrectionsTool::initialize() {
    m_corrected_indices_cache.resize(m_image_width * m_image_height, -1);
    for (int y_distorted = 0; y_distorted < m_image_height; y_distorted++) {
        for (int x_distorted = 0; x_distorted < m_image_width; x_distorted++) {
            const int index_distorted = y_distorted * m_image_width + x_distorted;
            const auto [x_corrected, y_corrected] = calculate_corrected_coordinates(x_distorted, y_distorted);
            const int x_corrected_rounded = static_cast<int>(std::round(x_corrected));
            const int y_corrected_rounded = static_cast<int>(std::round(y_corrected));
            if (x_corrected_rounded >= 0 && x_corrected_rounded < m_image_width &&
                y_corrected_rounded >= 0 && y_corrected_rounded < m_image_height) {
                const int index_corrected = y_corrected_rounded * m_image_width + x_corrected_rounded;
                m_corrected_indices_cache[index_distorted] = index_corrected;
            }
        }
    }
};

int LensCorrectionsTool::get_corrected_index(int distorted_index) const {
    if (m_corrected_indices_cache.empty()) {
        throw std::runtime_error("LensCorrectionsTool: get_corrected_index called before initialize()");
    }
    if (distorted_index < 0 || distorted_index >= static_cast<int>(m_corrected_indices_cache.size())) {
        return -1;
    }
    return m_corrected_indices_cache[distorted_index];
};


double LensCorrectionsTool::calculate_correction_scale_factor(double r_distorted_squared) const {
    return 1 + m_k1 * r_distorted_squared + m_k2 * r_distorted_squared * r_distorted_squared + m_k3 * r_distorted_squared * r_distorted_squared * r_distorted_squared;
};

double LensCorrectionsTool::calculate_correction_scale_factor(int x_distorted, int y_distorted) const {
    const double x_shifted = static_cast<double>(x_distorted) - m_center_x;
    const double y_shifted = static_cast<double>(y_distorted) - m_center_y;
    const double r_distorted_squared = x_shifted * x_shifted + y_shifted * y_shifted;
    return calculate_correction_scale_factor(r_distorted_squared);
};

std::pair<double, double> LensCorrectionsTool::calculate_corrected_coordinates(int x_distorted, int y_distorted) const {
    const double scale_factor = calculate_correction_scale_factor(x_distorted, y_distorted);
    const double x_shifted = static_cast<double>(x_distorted) - m_center_x;
    const double y_shifted = static_cast<double>(y_distorted) - m_center_y;
    const double x_corrected = m_center_x + x_shifted * scale_factor;
    const double y_corrected = m_center_y + y_shifted * scale_factor;
    return std::make_pair(x_corrected, y_corrected);
};
