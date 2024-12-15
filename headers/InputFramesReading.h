#pragma once

#include "../headers/InputFrame.h"
#include "../headers/raw_file_reader.h"

#include <string>
#include <vector>

namespace AstroPhotoStacker {

    /**
     * @brief Reads an image from a file and returns it as a 2D vector of pixels (first index - color, second index - x*width + y). In case of raw file, empty_pixel_value will be used for pixels with different color filters.

     * @tparam PixelType Type of pixel values in the image.
        * @param input_frame Input frame to read the image from.
        * @param width Pointer to the variable that will store the width of the image.
        * @param height Pointer to the variable that will store the height of the image.
        * @param empty_pixel_value Value to be used for pixels with different color filters in raw files.
     */
    template<typename PixelType>
    std::vector<std::vector<PixelType>> read_input_image(const InputFrame &input_frame, int *width, int *height, PixelType empty_pixel_value = -1) {
        const string &file_address = input_frame.get_file_address();
        if (is_raw_file(file_address))  {
            std::vector<char> colors;
            const std::vector<PixelType> data_original = read_raw_file<PixelType>(input_frame, width, height, &colors);
            const std::vector<char> color_conversion_table = get_color_info_as_number(input_frame);
            std::vector<std::vector<PixelType>> result(3, std::vector<PixelType>(data_original->size(), empty_pixel_value));
            for (unsigned i_pixel = 0; i_pixel < data_original.size(); i_pixel++) {
                result[color_conversion_table[i_pixel]][i_pixel] = data_original[i_pixel];
            }
            return result;
        }
        else {
            return read_rgb_image<PixelType>(input_frame, &m_width, &m_height);
        }
    }
}