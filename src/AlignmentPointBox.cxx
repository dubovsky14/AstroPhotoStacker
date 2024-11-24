#include "../headers/AlignmentPointBox.h"

#include <limits>
#include <cmath>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;


float AlignmentPointBox::s_contrast_threshold = 0.7;

AlignmentPointBox::AlignmentPointBox(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height, unsigned short max_value)   {
    m_x_center = x_center;
    m_y_center = y_center;
    m_box_width  = box_width % 2 == 1  ? box_width  : box_width  + 1;
    m_box_height = box_height % 2 == 1 ? box_height : box_height + 1;
    m_max_value = max_value;
    m_brightness = vector<unsigned short>(m_box_width*m_box_height);


    const unsigned int y_min = y_center - m_box_height/2;
    const unsigned int x_min = x_center - m_box_width/2;


    for (unsigned int y = 0; y < m_box_height; y++) {
        for (unsigned int x = 0; x < m_box_width; x++) {
            m_brightness[y * m_box_width + x] = image_data.brightness[(y_min + y) * image_data.width + x_min + x];
        }
    }

    m_max_acceptable_chi2 = calculate_acceptable_chi2(image_data);
};

float AlignmentPointBox::get_chi2(const MonochromeImageData &image_data, int x_center, int y_center) const  {
    double chi2 = 0;

    const unsigned int y_min = y_center - m_box_height/2;
    const unsigned int x_min = x_center - m_box_width/2;

    if (y_min < 0 || int(y_min + m_box_height) >= image_data.height || x_min < 0 || int(x_min + m_box_width) >= image_data.width) {
        throw runtime_error("AlignmentPointBox::get_chi2: Box out of bounds.");
    }

    for (unsigned int y = 0; y < m_box_height; y++) {
        for (unsigned int x = 0; x < m_box_width; x++) {
            const double brightness_input = image_data.brightness[(y_min + y) * image_data.width + x_min + x];
            const double brightness_box = m_brightness[y * m_box_width + x];
            chi2 += (brightness_input - brightness_box) * (brightness_input - brightness_box);
        }
    }

    return chi2;
};

bool AlignmentPointBox::is_valid_ap(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height, unsigned short max_value)    {
    unsigned short min_brightness = numeric_limits<unsigned short>::max();
    unsigned short max_brightness = numeric_limits<unsigned short>::min();

    int y_min = y_center - box_height/2;
    int y_max = y_center + box_height/2;
    int x_min = x_center - box_width/2;
    int x_max = x_center + box_width/2;

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
    for (unsigned short brightness : m_brightness) {
        sum += brightness;
        sum_sq += brightness*brightness;
    }

    const double mean = sum/m_brightness.size();
    const double rms = sqrt(sum_sq/m_brightness.size() - mean*mean);
    return rms/mean;
};

bool AlignmentPointBox::good_match(float chi2) const   {
    return chi2 < m_max_acceptable_chi2;
};

float AlignmentPointBox::get_sharpness_factor(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height)  {
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

    if (y_min < 0 || y_max >= image_data.height || x_min < 0 || x_max >= image_data.width) {
        return 0;
    }

    float sum = 0;
    for (int y = y_min; y < y_max; y++) {
        for (int x = x_min; x < x_max; x++) {
            const float sharpness_x = image_data.brightness[y*image_data.width + x + 1] - image_data.brightness[y*image_data.width + x];
            const float sharpness_y = image_data.brightness[(y+1)*image_data.width + x] - image_data.brightness[y*image_data.width + x];
            sum += sharpness_x*sharpness_x + sharpness_y*sharpness_y;
        }
    }
    return sum/((box_width-1)*(box_height-1));
};

float AlignmentPointBox::calculate_acceptable_chi2(const MonochromeImageData &image_data) const {
    const int max_shift = 2;
    float result = 0;
    for (int y_shift = -max_shift; y_shift <= max_shift; y_shift++) {
        for (int x_shift = -max_shift; x_shift <= max_shift; x_shift++) {
            result = max(result, get_chi2(image_data, m_x_center + x_shift, m_y_center + y_shift));
        }
    }
    return result;
};