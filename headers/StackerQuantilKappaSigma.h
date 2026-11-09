#pragma once

#include "../headers/StackerKappaSigmaBase.h"

namespace AstroPhotoStacker {

    /**
     * @brief Class for stacking photos using firstly quantil-based cut-off followed by Kappa-Sigma mean value
     */
    class StackerQuantilKappaSigma : public StackerKappaSigmaBase {
        public:

            /**
             * @brief Construct a new Stacker Quantil Kappa Sigma object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
            */
            StackerQuantilKappaSigma(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:
            virtual double get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) override;

            float m_central_quantil = 0.9f;
    };
}