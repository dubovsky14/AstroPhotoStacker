#pragma once

#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/InputFrame.h"
#include "../headers/AlignmentWindow.h"

#include <tuple>

namespace AstroPhotoStacker {
    template<class ValueType>
    float get_sharpness_factor(const ValueType *image_data, unsigned int width, unsigned int height, const AlignmentWindow &alignment_window = {-1,-1,-1,-1})    {
        double sum_weights          = 0;
        double sum_diff2_weighted   = 0;

        const bool use_alignment_window = alignment_window.x_min >= 0 && alignment_window.x_max >= 0 && alignment_window.y_min >= 0 && alignment_window.y_max >= 0;
        const unsigned int x0 = use_alignment_window ? alignment_window.x_min : 0;
        const unsigned int y0 = use_alignment_window ? alignment_window.y_min : 0;
        const unsigned int x1 = use_alignment_window ? alignment_window.x_max : width;
        const unsigned int y1 = use_alignment_window ? alignment_window.y_max : height;

        for (unsigned int y = y0; y < y1; y++)    {
            for (unsigned int x = x0; x < x1; x++)    {
                // right
                if (x < width - 1)  {
                    const double diff = fabs(double(image_data[y * width + x]) - image_data[y * width + x + 1]);
                    const double weight = std::max(image_data[y * width + x], image_data[y * width + x + 1]);
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }

                // down
                if (y < height - 1)  {
                    const double diff = fabs(double(image_data[y * width + x]) - image_data[(y + 1) * width + x]);
                    const double weight = fabs(std::max(image_data[y * width + x], image_data[(y + 1) * width + x]));
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }
            }
        }
        return sum_diff2_weighted / sum_weights;
    };


    float get_sharpness_for_file(const InputFrame &input_frame, const AlignmentWindow &alignment_window = {-1,-1,-1,-1});

}