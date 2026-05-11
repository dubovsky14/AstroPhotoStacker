#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>

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
        std::vector<std::vector<float>> sharpened_image(original_image.size(), std::vector<float>(original_image[0].size(), 0));

        std::vector<float> kernel_values;
        const int kernel_size = kernel.size();
        for (unsigned int y = 0; y < kernel.size(); y++) {
            if (int(kernel[y].size()) != kernel_size) {
                throw std::runtime_error("Kernel must be square");
            }
            for (unsigned int x = 0; x < kernel.size(); x++) {
                kernel_values.push_back(kernel[y][x]);
            }
        }

        const int kernel_half_size = kernel_size / 2;
        for (int y_kernel = 0; y_kernel < kernel_size; y_kernel++) {
            for (int i_color = 0; i_color < 3; i_color++) {
                for (int y = kernel_half_size; y < height - kernel_half_size; y++) {
                    for (int x_kernel = 0; x_kernel < kernel_size; x_kernel++) {
                        for (int x = kernel_half_size; x < width - kernel_half_size; x++) {
                            sharpened_image[i_color][y * width + x] += kernel_values[y_kernel * kernel_size + x_kernel] * original_image[i_color][(y + y_kernel - kernel_half_size) * width + x + x_kernel - kernel_half_size];
                        }
                    }
                }
            }
        }

        std::vector<std::vector<PixelType>> sharpened_image_return(original_image.size(), std::vector<PixelType>(original_image[0].size(), 0));
        for (int i_color = 0; i_color < 3; i_color++) {
            const int n_pixels = width * height;
            for (int i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
                sharpened_image_return[i_color][i_pixel] = std::max<float>(sharpened_image[i_color][i_pixel], 0);
            }
        }
        return sharpened_image_return;
    };


    #ifdef USE_CUDA
        std::vector<float> apply_kernel_cuda_float(const std::vector<float> &original_image, int width, int height, const std::vector<std::vector<float>> &kernel);

        template<typename PixelType>
        std::vector<std::vector<PixelType>> apply_kernel_cuda(const std::vector<std::vector<PixelType>> &original_image, int width, int height, const std::vector<std::vector<float>> &kernel)  {
            const size_t n_pixels = width * height;
            std::vector<std::vector<float>> original_image_float(3, std::vector<float>(n_pixels));
            for (size_t i_color = 0; i_color < original_image.size(); i_color++) {
                for (size_t i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
                    original_image_float[i_color][i_pixel] = static_cast<float>(original_image[i_color][i_pixel]);
                }
            }
            std::vector<std::vector<float>> sharpened_image_float;
            for (const std::vector<float> &color_channel : original_image_float) {
                sharpened_image_float.push_back(apply_kernel_cuda_float(color_channel, width, height, kernel));
            }

            std::vector<std::vector<PixelType>> sharpened_image_return(original_image.size(), std::vector<PixelType>(original_image[0].size(), 0));
            for (size_t i_color = 0; i_color < sharpened_image_float.size(); i_color++) {
                for (size_t i_pixel = 0; i_pixel < n_pixels; i_pixel++) {
                    sharpened_image_return[i_color][i_pixel] = static_cast<PixelType>(sharpened_image_float[i_color][i_pixel]);
                }
            }
            return sharpened_image_return;
        };
    #endif

    template<typename PixelType>
    std::vector<std::vector<PixelType>> sharpen_image(const std::vector<std::vector<PixelType>> &original_image, int width, int height, int kernel_size, float gauss_width, float center_value)  {
        const std::vector<std::vector<float>> kernel = get_sharpenning_kernel(kernel_size, gauss_width, center_value);
        #ifdef USE_CUDA
            try {
                return apply_kernel_cuda(original_image, width, height, kernel);
            }
            catch (const std::runtime_error &e) {
                std::cerr << "CUDA error: " << e.what() << std::endl;
                std::cerr << "Falling back to CPU implementation." << std::endl;
                return apply_kernel(original_image, width, height, kernel);
            }
        #else
            return apply_kernel(original_image, width, height, kernel);
        #endif
    };
}
