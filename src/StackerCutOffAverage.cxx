#include "../headers/StackerCutOffAverage.h"


StackerCutOffAverage::StackerCutOffAverage(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerMedian(number_of_colors, width, height, interpolate_colors)    {};

double StackerCutOffAverage::get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    unsigned int pixels_to_cut_off = int(m_tail_fraction_to_cut_off*number_of_stacked_pixels + 0.5);
    if (pixels_to_cut_off*2 >= number_of_stacked_pixels) {
        pixels_to_cut_off = 0;
    }

    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }

    double sum = 0;
    for (int i = pixels_to_cut_off; i < int(number_of_stacked_pixels - pixels_to_cut_off); i++) {
        sum += ordered_array_begin[i];
    }
    return sum/(number_of_stacked_pixels - 2*pixels_to_cut_off);
};
