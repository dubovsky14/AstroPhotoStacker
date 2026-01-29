#include "../headers/StackerQuantil.h"


StackerQuantil::StackerQuantil(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerMedian(number_of_colors, width, height, interpolate_colors)    {

    m_configurable_algorithm_settings.add_additional_setting_numerical("quantil_fraction", &m_quantil_fraction, 0.0, 0.45, 0.01);
};

double StackerQuantil::get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    const unsigned int selected_pixel = int(m_quantil_fraction*number_of_stacked_pixels + 0.5);
    if (number_of_stacked_pixels <= selected_pixel) {
        return c_empty_pixel_value;
    }

    return ordered_array_begin[selected_pixel];
};
