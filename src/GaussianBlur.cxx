#include "../headers/GaussianBlur.h"

#include <cmath>
#include <stdexcept>


using namespace std;
using namespace AstroPhotoStacker;

std::vector<unsigned short> AstroPhotoStacker::gaussian_blur(const MonochromeImageData &input_image, int blur_width, int blur_height, float sigma) {
    std::vector<unsigned short> output_image(input_image.width*input_image.height, 0);

    // Calculate the kernel
    const std::vector<double> kernel = get_gaussian_kernel(blur_width, blur_height, sigma);

    const int half_kernel_width = blur_width / 2;
    const int half_kernel_height = blur_height / 2;

    // Apply the kernel
    for (int i_x_image = half_kernel_width; i_x_image < input_image.width - half_kernel_width; i_x_image++) {
        for (int i_y_image = half_kernel_height; i_y_image < input_image.height - half_kernel_height; i_y_image++) {
            double sum = 0;
            for (int i_x_kernel = 0; i_x_kernel < blur_width; i_x_kernel++) {
                for (int i_y_kernel = 0; i_y_kernel < blur_height; i_y_kernel++) {
                    int i_x_image_kernel = i_x_image + i_x_kernel - blur_width / 2;
                    int i_y_image_kernel = i_y_image + i_y_kernel - blur_height / 2;
                    if (i_x_image_kernel >= 0 && i_x_image_kernel < input_image.width && i_y_image_kernel >= 0 && i_y_image_kernel < input_image.height) {
                        sum += input_image.brightness[i_y_image_kernel * input_image.width + i_x_image_kernel] * kernel[i_y_kernel * blur_width + i_x_kernel];
                    }
                }
            }
            output_image[i_y_image * input_image.width + i_x_image] = sum;
        }
    }

    return output_image;
}


std::vector<double> AstroPhotoStacker::get_gaussian_kernel(int width, int height, float sigma)    {
    if (width % 2 == 0 || height % 2 == 0) {
        throw std::invalid_argument("Width and height of the kernel must be odd.");
    }

    if (sigma <= 0) {
        throw std::invalid_argument("Sigma must be positive.");
    }

    if (width < 3 || height < 3) {
        throw std::invalid_argument("Width and height of the kernel must be at least 3.");
    }

    std::vector<double> kernel(width * height, 0);

    double sum = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double x = j - width / 2;
            double y = i - height / 2;
            kernel[i * width + j] = exp(-(x * x + y * y) / (2 * sigma * sigma));
            sum += kernel[i * width + j];
        }
    }

    // Normalize the kernel
    for (double &value : kernel) {
        value /= sum;
    }

    return kernel;
};
