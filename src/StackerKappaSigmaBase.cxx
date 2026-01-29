#include "../headers/StackerKappaSigmaBase.h"

#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaBase::StackerKappaSigmaBase(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerMedian(number_of_colors, width, height, interpolate_colors)    {

        m_configurable_algorithm_settings.add_additional_setting_numerical("kappa", &m_kappa, 0.1, 10.0, 0.1);
        m_configurable_algorithm_settings.add_additional_setting_numerical("n_iterations", &m_n_iterations, 1, 20, 1);
};


void StackerKappaSigmaBase::apply_kappa_sigma_clipping(PixelType **ordered_array_begin, unsigned int *number_of_stacked_pixels)    {
    PixelType * const current_begin = *ordered_array_begin;
    for (int i_iter = 0; i_iter < m_n_iterations; i_iter++)    {

        double mean(0), mean2(0);
        int n = 0;
        for (unsigned int i_pixel = 0; i_pixel < *number_of_stacked_pixels; i_pixel++) {
            if (current_begin[i_pixel] >= 0) {
                mean += current_begin[i_pixel];
                mean2 += current_begin[i_pixel]*current_begin[i_pixel];
                n++;
            }
        }
        mean /= n;
        mean2 /= n;
        const double sigma = sqrt(mean2 - mean*mean);
        const double kappa_sigma = m_kappa*sigma;
        for (unsigned int i_pixel = 0; i_pixel < *number_of_stacked_pixels; i_pixel++) {
            if (current_begin[i_pixel] >= 0) {
                if (abs(current_begin[i_pixel] - mean) > kappa_sigma) {
                    current_begin[i_pixel] = -1;
                }
            }
        }
    }

    sort(*ordered_array_begin, *ordered_array_begin + *number_of_stacked_pixels);

    // adjust new number of pixels and begin of array
    unsigned int new_number_of_stacked_pixels = 0;
    for (unsigned int i_pixel = 0; i_pixel < *number_of_stacked_pixels; i_pixel++) {
        if (current_begin[i_pixel] >= 0) {
            new_number_of_stacked_pixels++;
        }
    }
    *ordered_array_begin = *ordered_array_begin + *number_of_stacked_pixels - new_number_of_stacked_pixels;
    *number_of_stacked_pixels = new_number_of_stacked_pixels;
};