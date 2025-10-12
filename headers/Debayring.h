#pragma once

#include <vector>
#include <array>

#include "../headers/PixelType.h"

namespace AstroPhotoStacker {
    void get_closest_pixel_of_given_color(  const std::vector<PixelType> &data_original,
                                            const std::array<char, 4> &bayer_pattern,
                                            int height,
                                            int width,
                                            int pos_x,
                                            int pos_y,
                                            int color,
                                            int step_x,
                                            int step_y,
                                            PixelType *value,
                                            int *closest_distance,
                                            int n_steps_max = 2);

    std::vector<std::vector<PixelType>> debayer_raw_data(const std::vector<PixelType> &data_original, int width, int height, const std::array<char, 4> &bayer_pattern);

    void debayer_monochrome(std::vector<PixelType> *data, int width, int height, const std::array<char, 4> &bayer_pattern);
};
