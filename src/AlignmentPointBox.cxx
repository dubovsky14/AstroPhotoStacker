#include "../headers/AlignmentPointBox.h"

#include <limits>

using namespace std;
using namespace AstroPhotoStacker;


AlignmentPointBox::AlignmentPointBox(const MonochromeImageData &image_data, int x0, int y0, unsigned int size_x, unsigned int size_y, unsigned short max_value)   {
    m_brightness = vector<unsigned short>(size_x * size_y);
    m_x0 = x0;
    m_y0 = y0;
    m_size_x = size_x;
    m_size_y = size_y;
    m_max_value = max_value;

    for (unsigned int y = 0; y < size_y; y++) {
        for (unsigned int x = 0; x < size_x; x++) {
            m_brightness[y * size_x + x] = image_data.brightness[(y0 + y) * image_data.width + x0 + x];
        }
    }
};

float AlignmentPointBox::get_chi2(const MonochromeImageData &image_data, int window_pos_x, int window_pos_y) const  {
    double chi2 = 0;

    for (unsigned int y = 0; y < m_size_y; y++) {
        for (unsigned int x = 0; x < m_size_x; x++) {
            const double brightness_input = image_data.brightness[(window_pos_y + y) * image_data.width + window_pos_x + x];
            const double brightness_box = m_brightness[y * m_size_x + x];
            chi2 += (brightness_input - brightness_box) * (brightness_input - brightness_box);
        }
    }

    return chi2;
};

bool AlignmentPointBox::is_valid_ap(const MonochromeImageData &image_data, int x0, int y0, unsigned int size_x, unsigned int size_y, unsigned short max_value)    {
    unsigned short min_brightness = numeric_limits<unsigned short>::max();
    unsigned short max_brightness = numeric_limits<unsigned short>::min();

    for (unsigned int y = 0; y < size_y; y++) {
        for (unsigned int x = 0; x < size_x; x++) {
            const unsigned short brightness = image_data.brightness[(y0 + y) * image_data.width + x0 + x];
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