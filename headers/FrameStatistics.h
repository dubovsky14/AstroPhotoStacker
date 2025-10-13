#pragma once

#include "../headers/InputFrame.h"
#include "../headers/InputFrameReader.h"

#include <vector>
#include <numeric>
#include <cmath>
#include <iostream>

namespace AstroPhotoStacker {
    struct FrameStatistics  {
        unsigned int brightness_min = 2e9;
        unsigned int brightness_max = 0;
        float        brightness_avg = 0;
        float        brightness_std = 0;
        bool is_valid = false;
    };

    template<typename PixelValueType>
    FrameStatistics get_frame_statistics(const std::vector<std::vector<PixelValueType>>& image_data)    {
        FrameStatistics stats = {0, 0, 0, 0};
        if (image_data.empty()) return stats;

        unsigned long long sum_brightness           = 0;
        unsigned long long sum_brightness_squared   = 0;
        for (const std::vector<PixelValueType>& row : image_data) {
            for (const PixelValueType &pixel : row) {
                stats.brightness_min = std::min<unsigned int>(stats.brightness_min, pixel);
                stats.brightness_max = std::max<unsigned int>(stats.brightness_max, pixel);
                sum_brightness += pixel;
                sum_brightness_squared += pixel * static_cast<unsigned long long>(pixel);
            }
        }
        const double pixel_count = image_data.size() * image_data[0].size();

        stats.brightness_avg = static_cast<float>(sum_brightness) / pixel_count;
        stats.brightness_std = static_cast<float>(std::sqrt(sum_brightness_squared / pixel_count - stats.brightness_avg * stats.brightness_avg));
        stats.is_valid = true;

        return stats;
    };


    template<typename PixelValueType>
    FrameStatistics get_frame_statistics(const std::vector<PixelValueType>& image_data)    {
        FrameStatistics stats = {0, 0, 0, 0};
        if (image_data.empty()) return stats;

        unsigned long long sum_brightness           = 0;
        unsigned long long sum_brightness_squared   = 0;
        for (const PixelValueType &pixel : image_data) {
            stats.brightness_min = std::min<unsigned int>(stats.brightness_min, pixel);
            stats.brightness_max = std::max<unsigned int>(stats.brightness_max, pixel);
            sum_brightness += pixel;
            sum_brightness_squared += pixel * static_cast<unsigned long long>(pixel);
        }

        const double pixel_count = image_data.size();

        stats.brightness_avg = static_cast<float>(sum_brightness) / pixel_count;
        stats.brightness_std = static_cast<float>(std::sqrt(sum_brightness_squared / pixel_count - stats.brightness_avg * stats.brightness_avg));
        stats.is_valid = true;

        return stats;
    };


    inline FrameStatistics get_frame_statistics(const InputFrame &input_frame) {
        InputFrameReader input_frame_data(input_frame);
        input_frame_data.debayer();
        return get_frame_statistics(input_frame_data.get_rgb_data());
    }
}


