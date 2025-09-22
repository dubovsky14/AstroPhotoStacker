#pragma once

#include "../headers/StackerMedian.h"

namespace AstroPhotoStacker {

    /**
     * @brief Purely virtual base class for stacking algorithms that use the Kappa-Sigma clipping algorithm.
     *
     * The kappa-sigma clippling iteratively drops values more than kappa*sigma from mean value. Kappa and also the number of iterations can be set.
     */
    class StackerKappaSigmaBase : public StackerMedian {
    public:
            /**
             * @brief Construct a new Stacker Kappa Sigma Base object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
            */
            StackerKappaSigmaBase(int number_of_colors, int width, int height, bool interpolate_colors);

    protected:
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels)  = 0;

            /**
             * @brief Apply the Kappa-Sigma clipping algorithm to the ordered array of pixels
             *
             * @param ordered_array_begin - pointer to the first element of the ordered array
             * @param number_of_stacked_pixels - number of pixels to stack
             */
            void apply_kappa_sigma_clipping(short int **ordered_array_begin, unsigned int *number_of_stacked_pixels);

            float   m_kappa       = 3.0;
            int     m_n_iterations  = 3;
    };
}