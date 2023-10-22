#pragma once

#include "../headers/StackerKappaSigmaBase.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaMedian : public StackerKappaSigmaBase {
        public:
            StackerKappaSigmaMedian(int number_of_colors, int width, int height);

        protected:
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
    };
}