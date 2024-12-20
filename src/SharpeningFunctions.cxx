#include "../headers/SharpeningFunctions.h"

#include <cmath>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

vector<vector<float>> AstroPhotoStacker::get_sharpenning_kernel(int kernel_size, float gauss_width, float center_value) {
    if (kernel_size % 2 == 0) {
        throw runtime_error("Kernel size must be odd.");
    }
    vector<vector<float>> kernel(kernel_size, vector<float>(kernel_size, 0));
    const int center = kernel_size / 2;
    float normalization = 0;
    for (int y = 0; y < kernel_size; y++) {
        for (int x = 0; x < kernel_size; x++) {
            const float distance_from_center_squared = (y - center) * (y - center) + (x - center) * (x - center);
            kernel[y][x] = center_value*exp(-distance_from_center_squared / (2 * gauss_width * gauss_width));
            normalization += kernel[y][x];
        }
    }

    const float offsef = (normalization-1)/(kernel_size*kernel_size);

    for (int y = 0; y < kernel_size; y++) {
        for (int x = 0; x < kernel_size; x++) {
            kernel[y][x] -= offsef;
        }
    }

    return kernel;
}
