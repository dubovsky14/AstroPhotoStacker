#include "../headers/StackerKappaSigmaMedian.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaMedian::StackerKappaSigmaMedian(int number_of_colors, int width, int height) :
    StackerKappaSigmaBase(number_of_colors, width, height)   {
};

double StackerKappaSigmaMedian::get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    apply_kappa_sigma_clipping(&ordered_array_begin, &number_of_stacked_pixels);
    if (number_of_stacked_pixels == 0) {
        return 0;
    }
    else if (number_of_stacked_pixels % 2 == 0) {
        return (ordered_array_begin[number_of_stacked_pixels/2] + ordered_array_begin[number_of_stacked_pixels/2 - 1])/2;
    }
    else {
        return ordered_array_begin[number_of_stacked_pixels/2];
    }
};