#pragma once

#include <vector>

namespace AstroPhotoStacker {

    /**
     * @brief Get a sharpenning kernel
     *
     * @param kernel_size The size of the kernel - must be odd number
     * @param gauss_width The width of the Gaussian distribution
     * @param center_value The value in the center of the kernel
     * @return std::vector<std::vector<float>> The sharpenning kernel
     */
    std::vector<std::vector<float>> get_sharpenning_kernel(int kernel_size, float gauss_width, float center_value);

    template<typename PixelType>
    std::vector<std::vector<PixelType>> apply_kernel(const std::vector<std::vector<PixelType>> &original_image, int width, int height, const std::vector<std::vector<float>> &kernel)  {
        std::vector<std::vector<PixelType>> sharpened_image(original_image.size(), std::vector<PixelType>(original_image[0].size(), 0));

        const int kernel_size = kernel.size();
        const int kernel_half_size = kernel_size / 2;
        for (int y = kernel_half_size; y < height - kernel_half_size; y++) {
            for (int x = kernel_half_size; x < width - kernel_half_size; x++) {
                for (int i_color = 0; i_color < 3; i_color++) {
                    float sum = 0;
                    for (int y_kernel = 0; y_kernel < kernel_size; y_kernel++) {
                        for (int x_kernel = 0; x_kernel < kernel_size; x_kernel++) {
                            sum += kernel[y_kernel][x_kernel] * original_image[i_color][(y + y_kernel - kernel_half_size) * width + x + x_kernel - kernel_half_size];
                        }
                    }
                    sharpened_image[i_color][y * width + x] = std::max<float>(sum,0.);
                }
            }
        }
        return sharpened_image;
    };


    template<typename PixelType>
    std::vector<std::vector<PixelType>> sharpen_image(const std::vector<std::vector<PixelType>> &original_image, int width, int height, int kernel_size, float gauss_width, float center_value)  {
        const std::vector<std::vector<float>> kernel = get_sharpenning_kernel(kernel_size, gauss_width, center_value);
        return apply_kernel(original_image, width, height, kernel);
    };


}
