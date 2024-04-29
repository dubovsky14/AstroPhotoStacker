#pragma once

#include "../headers/StackerKappaSigmaBase.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaClipping : public StackerKappaSigmaBase {
        public:
            StackerKappaSigmaClipping(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
    };
}