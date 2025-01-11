#include "../headers/AlignmentPointBox.h"

#include <limits>
#include <cmath>
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;


float AlignmentPointBox::s_contrast_threshold = 0.7;

AlignmentPointBox::AlignmentPointBox(const std::vector<float> *scaled_data_vector,
                                    const AlignmentWindow &alignment_window,
                                    int x_center, int y_center,
                                    int box_width, int box_height, float max_value)   {

    m_scaled_data_vector = scaled_data_vector;
    m_alignment_window = alignment_window;
    m_alignment_window_width  = alignment_window.x_max - alignment_window.x_min;
    m_alignment_window_height = alignment_window.y_max - alignment_window.y_min;
    m_x_center = x_center;
    m_y_center = y_center;
    m_box_width  = box_width % 2 == 1  ? box_width  : box_width  + 1;
    m_box_height = box_height % 2 == 1 ? box_height : box_height + 1;
    m_max_value = max_value;

    const int y_min = y_center - m_box_height/2;
    const int x_min = x_center - m_box_width/2;

    m_x_min_in_alignment_window = x_min - alignment_window.x_min;
    m_y_min_in_alignment_window = y_min - alignment_window.y_min;

    m_max_acceptable_chi2 = calculate_acceptable_chi2();
};

float AlignmentPointBox::get_chi2(const std::vector<float> *scaled_data_vector, int x_center, int y_center) const  {
    double chi2 = 0;

    int y_min = y_center - m_box_height/2;
    int y_max = y_center + m_box_height/2;
    int x_min = x_center - m_box_width/2;
    int x_max = x_center + m_box_width/2;

    if (y_min < 0 || y_max >= m_alignment_window_height || x_min < 0 || x_max >= m_alignment_window_width) {
        cout << "AlignmentPointBox::get_chi2: Box out of bounds." << endl;
        throw runtime_error("AlignmentPointBox::get_chi2: Box out of bounds.");
    }

    for (int y = 0; y < m_box_height; y++) {
        for (int x = 0; x < m_box_width; x++) {
            const double brightness_input = (*scaled_data_vector)[(y_min + y) * m_alignment_window_width + x_min + x];
            const double brightness_box = (*m_scaled_data_vector)[(m_y_min_in_alignment_window + y) * m_alignment_window_width + m_x_min_in_alignment_window + x];
            chi2 += (brightness_input - brightness_box) * (brightness_input - brightness_box);
        }
    }

    return chi2;
};

bool AlignmentPointBox::is_valid_ap(const std::vector<float> *scaled_data_vector, const AlignmentWindow &alignment_window, int x_center, int y_center, int box_width, int box_height, float max_value)    {
    float min_brightness = 10e20;
    float max_brightness = -1;

    int y_min = y_center - box_height/2;
    int y_max = y_center + box_height/2;
    int x_min = x_center - box_width/2;
    int x_max = x_center + box_width/2;

    const int alignment_window_width = alignment_window.x_max - alignment_window.x_min;
    const int alignment_window_height = alignment_window.y_max - alignment_window.y_min;

    if (y_min < 0 || y_max >= alignment_window_height || x_min < 0 || x_max >= alignment_window_width) {
        return false;
    }

    int n_pixels_above_5_percent_of_maximum = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            const float brightness = (*scaled_data_vector)[y * alignment_window_width + x];
            if (brightness < min_brightness) {
                min_brightness = brightness;
            }
            if (brightness > max_brightness) {
                max_brightness = brightness;
            }
            if (brightness > 0.05*max_value) {
                n_pixels_above_5_percent_of_maximum++;
            }
        }
    }

    // at least 30% of the pixels should be above 5% of the maximum
    if (n_pixels_above_5_percent_of_maximum < 0.3*box_width*box_height) {
        return false;
    }

    if (max_brightness < 0.25*max_value) {
        return false;
    }

    if (float(min_brightness)/max_brightness > s_contrast_threshold) {
        return false;
    }

    return true;
};

void  AlignmentPointBox::set_contrast_threshold(float threshold)   {
    s_contrast_threshold = threshold;
};

float AlignmentPointBox::get_contrast_threshold()  {
    return s_contrast_threshold;
};

float AlignmentPointBox::get_relative_rms() const  {
    double sum = 0;
    double sum_sq = 0;
    for (int i_x = m_x_min_in_alignment_window; i_x < m_x_min_in_alignment_window + m_box_width; i_x++) {
        for (int i_y = m_y_min_in_alignment_window; i_y < m_y_min_in_alignment_window + m_box_height; i_y++) {
            const double brightness = (*m_scaled_data_vector)[i_y*m_alignment_window_width + i_x];
            sum += brightness;
            sum_sq += brightness*brightness;
        }
    }

    const double alignment_box_area = m_box_width*m_box_height;

    const double mean = sum/alignment_box_area;
    const double rms = sqrt(sum_sq/alignment_box_area - mean*mean);
    return rms/mean;
};

bool AlignmentPointBox::good_match(float chi2) const   {
    return chi2 < m_max_acceptable_chi2;
};

float AlignmentPointBox::get_sharpness_factor(const std::vector<float> *scaled_data_vector, const AlignmentWindow &alignment_window, int x_center, int y_center, int box_width, int box_height)  {
    if (box_width % 2 == 0) {
        box_width += 1;
    }
    if (box_height % 2 == 0) {
        box_height += 1;
    }

    int y_min = y_center - box_height/2;
    int y_max = y_center + box_height/2;
    int x_min = x_center - box_width/2;
    int x_max = x_center + box_width/2;

    const int alignment_window_width = alignment_window.x_max - alignment_window.x_min;
    const int alignment_window_height = alignment_window.y_max - alignment_window.y_min;

    if (y_min < 0 || y_max >= alignment_window_height || x_min < 0 || x_max >= alignment_window_width) {
        return false;
    }

    float sum = 0;
    for (int y = y_min; y < y_max-1; y++) {
        for (int x = x_min; x < x_max-1; x++) {
            const float sharpness_x = (*scaled_data_vector)[y*alignment_window_width + x + 1] - (*scaled_data_vector)[y*alignment_window_width + x];
            const float sharpness_y = (*scaled_data_vector)[(y+1)*alignment_window_width + x] - (*scaled_data_vector)[y*alignment_window_width + x];
            sum += sharpness_x*sharpness_x + sharpness_y*sharpness_y;
        }
    }
    return sum/((box_width-1)*(box_height-1));
};

bool AlignmentPointBox::valid_box_coordinates(int x_center, int y_center, int box_width, int box_height) const   {
    int y_min = y_center - box_height/2;
    int y_max = y_center + box_height/2;
    int x_min = x_center - box_width/2;
    int x_max = x_center + box_width/2;

    if (y_min < 0 || y_max >= m_alignment_window_height || x_min < 0 || x_max >= m_alignment_window_width) {
        return false;
    }
    return true;
};

float AlignmentPointBox::calculate_acceptable_chi2() const {
    const int max_shift = 4;
    float result = 0;
    for (int y_shift = -max_shift; y_shift <= max_shift; y_shift++) {
        for (int x_shift = -max_shift; x_shift <= max_shift; x_shift++) {
            const int x_shifted = get_center_x_in_alignment_window_coordinates() + x_shift;
            const int y_shifted = get_center_y_in_alignment_window_coordinates() + y_shift;
            if (!valid_box_coordinates(x_shifted, y_shifted, get_box_width(), get_box_height())) {
                continue;
            }
            result = max(result, get_chi2(m_scaled_data_vector, x_shifted, y_shifted));
        }
    }
    return result;
};