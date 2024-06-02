#include "../headers/StackerMaximum.h"


#include <iostream>

StackerMaximum::StackerMaximum(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerMedian(number_of_colors, width, height, interpolate_colors)    {
        std::cout << "StackerMaximum::StackerMaximum created" << std::endl;
    };

double StackerMaximum::get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }
    return ordered_array_begin[number_of_stacked_pixels - 1];
};
