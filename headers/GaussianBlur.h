#pragma once

#include "../headers/MonochromeImageData.h"

#include <vector>


namespace AstroPhotoStacker {
    std::vector<unsigned short> gaussian_blur(const MonochromeImageData &input_image, int blur_width, int blur_height, float sigma);

    std::vector<double> get_gaussian_kernel(int width, int height, float sigma);
}

