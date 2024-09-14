#pragma once

#include "../headers/CalibratedPhotoHandler.h"

namespace AstroPhotoStacker {
    template<class ValueType>
    float get_sharpness_factor(const ValueType *image_data, unsigned int width, unsigned int height)    {
        double sum_weights          = 0;
        double sum_diff2_weighted   = 0;

        for (unsigned int y = 0; y < height; y++)    {
            for (unsigned int x = 0; x < width; x++)    {
                // right
                if (x < width - 1)  {
                    const double diff = image_data[y * width + x] - image_data[y * width + x + 1];
                    const double weight = std::max(image_data[y * width + x], image_data[y * width + x + 1]);
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }

                // down
                if (y < height - 1)  {
                    const double diff = image_data[y * width + x] - image_data[(y + 1) * width + x];
                    const double weight = std::max(image_data[y * width + x], image_data[(y + 1) * width + x]);
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }
            }
        }
        return sum_diff2_weighted / sum_weights;
    }


    float get_sharpness_for_file(const std::string &input_file)  {
        CalibratedPhotoHandler calibrated_photo_handler(input_file, true);
        calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
        calibrated_photo_handler.calibrate();

        const std::vector<std::vector<short int>> &data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();
        const int width = calibrated_photo_handler.get_width();
        const int height = calibrated_photo_handler.get_height();

        float average_sharpness = 0;
        for (unsigned int i_color = 0; i_color < data.size(); i_color++)    {
            const float sharpness = AstroPhotoStacker::get_sharpness_factor(data[i_color].data(), width, height);
            average_sharpness += sharpness;
        }
        average_sharpness /= data.size();

        return average_sharpness;
    }

}