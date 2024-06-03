#pragma once

#include "../headers/StackerMedian.h"

/**
 * @brief Class for stacking photos using maximal value. This can be useful to stack star trails or thunderstorms.
 *
 * #TODO: This is quite dumb implementation - we do not need the whole array in the memory. Need to change it in a future.
 */
class StackerMaximum : public AstroPhotoStacker::StackerMedian {
    public:
        /**
         * @brief Construct a new Stacker Cut Off Average object
         *
         * @param number_of_colors - number of colors in the stacked photo
         * @param width - width of the photo
         * @param height - height of the photo
         * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
        */
        StackerMaximum(int number_of_colors, int width, int height, bool interpolate_colors);

    protected:
        virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
};