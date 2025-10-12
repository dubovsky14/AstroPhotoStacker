#pragma once

#include "../headers/TestUtils.h"

#include "../../headers/InputFrame.h"
#include "../../headers/PixelType.h"

#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker {
    /**
     * @brief Test function for reading image from raw files
     *
     * @param input_frame InputFrame object containing the file address (and frame number for videos)
     * @param expected_pixel_values Vector of tuples containing: (x-coordinate, y-coordinate, expected pixel value, expected color>
     */
    TestResult test_image_reading_raw(  const InputFrame &input_frame,
                                        const std::pair<int,int> &expected_resolution,
                                        const std::vector<std::tuple<int, int, PixelType, char>> &expected_pixel_values);
}