#pragma once

#include "../headers/StackerMedian.h"

/**
 * @brief Class for stacking photos using quantil method
 */
class StackerQuantil : public AstroPhotoStacker::StackerMedian {
    public:
        /**
         * @brief Construct a new Stacker Quantil object
         *
         * @param number_of_colors - number of colors in the stacked photo
         * @param width - width of the photo
         * @param height - height of the photo
         * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
        */
        StackerQuantil(int number_of_colors, int width, int height, bool interpolate_colors);

    protected:
        float m_quantil_fraction    = 0.3;

        virtual double get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
};