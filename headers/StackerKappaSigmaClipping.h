#pragma once

#include "../headers/StackerKappaSigmaBase.h"

namespace AstroPhotoStacker {

    /**
     * @brief Class for stacking photos using Kappa-Sigma mean value
     */
    class StackerKappaSigmaClipping : public StackerKappaSigmaBase {
        public:

            /**
             * @brief Construct a new Stacker Kappa Sigma Clipping object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
            */
            StackerKappaSigmaClipping(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
    };
}