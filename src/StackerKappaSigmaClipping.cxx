#include "../headers/StackerKappaSigmaClipping.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaClipping::StackerKappaSigmaClipping(int number_of_colors, int width, int height) :
    StackerKappaSigmaBase(number_of_colors, width, height)   {
};

double StackerKappaSigmaClipping::get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    apply_kappa_sigma_clipping(&ordered_array_begin, &number_of_stacked_pixels);
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }
    double result = 0;
    for (unsigned int i = 0; i < number_of_stacked_pixels; i++) {
        result += ordered_array_begin[i];
    }
    return result/number_of_stacked_pixels;
};