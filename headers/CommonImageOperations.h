#pragma once

#include <vector>

namespace AstroPhotoStacker {

    /**
     * @brief Calculate Otsu's threshold for a given monochrome image
     *
     * @param brightness Pointer to the brightness pixel data
     * @param n_pixels Number of pixels in the image
     * @return unsigned short The calculated Otsu's threshold
    */
    unsigned short get_otsu_threshold(const unsigned short *brightness, int n_pixels, bool *contains_only_one_value = nullptr);

    template <typename PixelValueTypeInput, typename PixelValueTypeOutput = PixelValueTypeInput>
    std::vector<PixelValueTypeOutput> convert_color_to_monochrome(const std::vector<std::vector<PixelValueTypeInput>> &color_image, int width, int height) {
        const unsigned int n_pixels = width * height;
        const unsigned int n_colors = color_image.size();

        std::vector<float> temp_result(n_pixels,0);
        for (unsigned int i_color = 0; i_color < n_colors; i_color++) {
            const std::vector<PixelValueTypeInput> &color_channel = color_image[i_color];
            for (unsigned int i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
                temp_result[i_pixel] += color_channel[i_pixel];
            }
        }

        std::vector<PixelValueTypeOutput> result(n_pixels);
        for (unsigned int i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
            result[i_pixel] = static_cast<PixelValueTypeOutput>(temp_result[i_pixel] / n_colors);
        }
        return result;
    }

    template<typename InputPixelType>
    std::vector<std::vector<unsigned short>> scale_image_to_16bit_uint(const std::vector<std::vector<InputPixelType>> &input_image) {
        std::vector<std::vector<unsigned short>> result;
        InputPixelType max_value = 0;
        for (const std::vector<InputPixelType> &input_channel : input_image) {
            for (InputPixelType value : input_channel) {
                if (value > max_value) {
                    max_value = value;
                }
            }
            result.push_back(std::vector<unsigned short>(input_channel.size()));
        }
        const double scale_factor = 65535.0f / max_value;

        for (size_t i_channel = 0; i_channel < input_image.size(); i_channel++) {
            const std::vector<InputPixelType> &input_channel = input_image[i_channel];
            std::vector<unsigned short> &output_channel = result[i_channel];
            for (size_t i = 0; i < input_channel.size(); i++) {
                output_channel[i] = static_cast<unsigned short>(input_channel[i] * scale_factor);
            }
        }
        return result;
    };

    template<typename InputPixelType>
    void scale_image_to_maximum(std::vector<std::vector<InputPixelType>> *input_image, double desired_max_value) {
        InputPixelType max_value = 0;
        for (const std::vector<InputPixelType> &input_channel : *input_image) {
            for (InputPixelType value : input_channel) {
                if (value > max_value) {
                    max_value = value;
                }
            }
        }
        const double scale_factor = double(desired_max_value) / max_value;

        for (size_t i_channel = 0; i_channel < input_image->size(); i_channel++) {
            std::vector<InputPixelType> &input_channel = (*input_image)[i_channel];
            for (size_t i = 0; i < input_channel.size(); i++) {
                input_channel[i] = static_cast<InputPixelType>(input_channel[i] * scale_factor);
            }
        }
    };

}