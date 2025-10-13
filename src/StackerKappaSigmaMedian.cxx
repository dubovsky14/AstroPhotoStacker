#include "../headers/StackerKappaSigmaMedian.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaMedian::StackerKappaSigmaMedian(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerKappaSigmaBase(number_of_colors, width, height, interpolate_colors)   {
};

double StackerKappaSigmaMedian::get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    apply_kappa_sigma_clipping(&ordered_array_begin, &number_of_stacked_pixels);
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }
    else if (number_of_stacked_pixels % 2 == 0) {
        return (ordered_array_begin[number_of_stacked_pixels/2] + ordered_array_begin[number_of_stacked_pixels/2 - 1])/2;
    }
    else {
        return ordered_array_begin[number_of_stacked_pixels/2];
    }
};