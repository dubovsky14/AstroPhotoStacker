#include "../headers/AlignmentPointBox.h"

#include <limits>
#include <cmath>
using namespace std;
using namespace AstroPhotoStacker;


float AlignmentPointBox::s_contrast_threshold = 0.8;

AlignmentPointBox::AlignmentPointBox(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_size, unsigned short max_value)   {
    m_x_center = x_center;
    m_y_center = y_center;
    m_box_size = box_size % 2 == 1 ? box_size : box_size + 1;
    m_max_value = max_value;
    m_brightness = vector<unsigned short>(m_box_size*m_box_size);


    unsigned int y_min = y_center - m_box_size/2;
    unsigned int x_min = x_center - m_box_size/2;


    for (unsigned int y = 0; y < m_box_size; y++) {
        for (unsigned int x = 0; x < m_box_size; x++) {
            m_brightness[y * m_box_size + x] = image_data.brightness[(y_min + y) * image_data.width + x_min + x];
        }
    }
};

float AlignmentPointBox::get_chi2(const MonochromeImageData &image_data, int x_center, int y_center) const  {
    double chi2 = 0;

    const int half_size = m_box_size/2;
    for (int y_shift = -half_size; y_shift <= half_size; y_shift++) {
        for (int x_shift = -half_size; x_shift <= half_size/2; x_shift++) {
            const double brightness_input = image_data.brightness[(y_center + y_shift) * image_data.width + (x_center + x_shift)];
            const double brightness_box = m_brightness[(half_size + y_shift) * m_box_size + (half_size + x_shift)];
            chi2 += (brightness_input - brightness_box) * (brightness_input - brightness_box);
        }
    }

    return chi2;
};

bool AlignmentPointBox::is_valid_ap(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_size, unsigned short max_value)    {
    unsigned short min_brightness = numeric_limits<unsigned short>::max();
    unsigned short max_brightness = numeric_limits<unsigned short>::min();

    int y_min = y_center - box_size/2;
    int y_max = y_center + box_size/2;
    int x_min = x_center - box_size/2;
    int x_max = x_center + box_size/2;

    if (y_min < 0 || y_max >= image_data.height || x_min < 0 || x_max >= image_data.width) {
        return false;
    }

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            const unsigned short brightness = image_data.brightness[y * image_data.width + x];
            if (brightness < min_brightness) {
                min_brightness = brightness;
            }
            if (brightness > max_brightness) {
                max_brightness = brightness;
            }
        }
    }

    if (max_brightness < 0.3*max_value) {
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
    for (unsigned short brightness : m_brightness) {
        sum += brightness;
        sum_sq += brightness*brightness;
    }

    const double mean = sum/m_brightness.size();
    const double rms = sqrt(sum_sq/m_brightness.size() - mean*mean);
    return rms/mean;
};

bool AlignmentPointBox::good_match(float chi2) const   {
    return true; // #TODO: Implement this
};