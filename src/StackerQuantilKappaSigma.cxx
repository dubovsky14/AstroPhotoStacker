#include "../headers/StackerQuantilKappaSigma.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerQuantilKappaSigma::StackerQuantilKappaSigma(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerKappaSigmaBase(number_of_colors, width, height, interpolate_colors)   {

    m_configurable_algorithm_settings.add_additional_setting_numerical("central_quantil", &m_central_quantil, 0.0, 1.0, 0.01);
};

double StackerQuantilKappaSigma::get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    int tails_to_cut = static_cast<int>(number_of_stacked_pixels * (1.0 - m_central_quantil) / 2.0);
    size_t start_index = tails_to_cut;
    size_t end_index = number_of_stacked_pixels - tails_to_cut;
    ordered_array_begin += start_index;
    number_of_stacked_pixels = end_index - start_index;

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