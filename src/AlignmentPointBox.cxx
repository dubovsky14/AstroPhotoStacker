#include "../headers/AlignmentPointBox.h"

#include <limits>

using namespace std;
using namespace AstroPhotoStacker;


AlignmentPointBox::AlignmentPointBox(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_size, unsigned short max_value)   {
    m_x_center = x_center;
    m_y_center = y_center;
    m_box_size = box_size % 2 == 1 ? box_size : box_size + 1;
    m_max_value = max_value;
    m_brightness = vector<unsigned short>(box_size*box_size);


    unsigned int y_min = y_center - m_box_size/2;
    unsigned int y_max = y_center + m_box_size/2;
    unsigned int x_min = x_center - m_box_size/2;
    unsigned int x_max = x_center + m_box_size/2;


    for (unsigned int y = y_min; y <= y_max; y++) {
        for (unsigned int x = x_min; x <= x_max; x++) {
            m_brightness[y * m_box_size + x] = image_data.brightness[(y_center + y) * image_data.width + x_center + x];
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
    int x_max = x_center - box_size/2;
    int x_min = x_center + box_size/2;

    if (y_min < 0 || y_max >= image_data.height || x_min < 0 || x_max >= image_data.width) {
        return false;
    }

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            const unsigned short brightness = image_data.brightness[(y_center + y) * image_data.width + x_center + x];
            if (brightness < min_brightness) {
                min_brightness = brightness;
            }
            if (brightness > max_brightness) {
                max_brightness = brightness;
            }
        }
    }

    // check if there is enough structure in the box to be able to compute alignment
    if (max_brightness < 0.4*max_value) {
        return false;
    }

    if (max_brightness/float(min_brightness) < 1.5) {
        return false;
    }

    return true;
};