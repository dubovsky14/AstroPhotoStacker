#pragma once

#include "../../headers/LocalShift.h"

#include <vector>

template<typename PixelType>
void draw_alignment_points_into_image(  const std::vector<AstroPhotoStacker::LocalShift> &alignment_points,
                                    std::vector<std::vector<PixelType>> *image,
                                    int width,
                                    int height,
                                    const std::vector<int> &valid_ap_color,
                                    const std::vector<int> &invalid_ap_color)    {

    for (const AstroPhotoStacker::LocalShift &point : alignment_points) {
        const int x = static_cast<int>(point.x);
        const int y = static_cast<int>(point.y);
        const float radius = 5.0;
        const int x_min = std::max(0, static_cast<int>(x - radius));
        const int x_max = std::min(width - 1, static_cast<int>(x + radius));
        const int y_min = std::max(0, static_cast<int>(y - radius));
        const int y_max = std::min(height - 1, static_cast<int>(y + radius));

        const float radius_squared = radius * radius;
        for (int i_color = 0; i_color < 3; i_color++) {
            for (int y = y_min; y <= y_max; y++) {
                for (int x = x_min; x <= x_max; x++) {
                    const float distance_squared = (x - point.x)*(x - point.x) + (y - point.y)*(y - point.y);
                    if (distance_squared > radius_squared) {
                        continue;
                    }
                    (*image)[i_color][y*width + x] = point.valid_ap ? valid_ap_color[i_color] : invalid_ap_color[i_color];
                }
            }
        }
    }
};
