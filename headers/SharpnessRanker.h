#pragma once

#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/InputFrame.h"
#include "../headers/AlignmentWindow.h"

#include <tuple>

namespace AstroPhotoStacker {
    template<class ValueType>
    float get_sharpness_factor(const ValueType *image_data, unsigned int width, unsigned int height, const AlignmentWindow &alignment_window = {-1,-1,-1,-1})    {
        const bool use_alignment_window = alignment_window.x_min >= 0 && alignment_window.x_max >= 0 && alignment_window.y_min >= 0 && alignment_window.y_max >= 0;
        const unsigned int x0 = use_alignment_window ? alignment_window.x_min : 0;
        const unsigned int y0 = use_alignment_window ? alignment_window.y_min : 0;
        const unsigned int x1 = use_alignment_window ? alignment_window.x_max : width;
        const unsigned int y1 = use_alignment_window ? alignment_window.y_max : height;

        double mean = 0;
        double mean2 = 0;
        for (unsigned int y = y0; y < y1; y++)    {
            for (unsigned int x = x0; x < x1; x++)    {
                mean += image_data[y * width + x];
                mean2 += image_data[y * width + x] * image_data[y * width + x];
            }
        }
        const double n_pixels = (x1 - x0) * (y1 - y0);
        mean /= n_pixels;
        mean2 /= n_pixels;

        const double rms = sqrt(mean2 - mean*mean);
        return rms;
    };


    float get_sharpness_for_file(const InputFrame &input_frame, const AlignmentWindow &alignment_window = {-1,-1,-1,-1});

}