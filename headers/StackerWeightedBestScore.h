#pragma once

#include "../headers/StackerWeightedMedian.h"

#include <tuple>

namespace AstroPhotoStacker {
    /**
     * @brief Stacking algorithm that for each pixel select the value with the highest score
     */
    class StackerWeightedBestScore : public StackerWeightedMedian {
        public:
            StackerWeightedBestScore(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:

            /**
             * @brief The method defines how to stack "number_of_stacked pixels" values from pixels in individual photos into one file. In case of this class it is a median. Can be overriden in derived classes to implement different stacking algorithms.
             *
             * @param ordered_array_begin       - pointer ot the first element of the ordered array
             * @param number_of_stacked_pixels  - number of pixels to stack
             */
            virtual double get_stacked_value_from_pixel_array(std::tuple<PixelType,ScoreType> *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
    };
}