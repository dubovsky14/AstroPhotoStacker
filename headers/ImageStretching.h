#pragma once

#include <cmath>
#include <vector>


namespace AstroPhotoStacker {
    enum class StretchingType{logarithmic, quadratic, linear, linear_and_logarithmic};

    double get_scale_factor(double pixel_value, double max_value, StretchingType stretching_type);

    void stretch_image(std::vector<std::vector<double> > *image, double max_value, StretchingType stretching_type, bool keep_rgb_ratio = true);

    template <typename input_array_type>
    std::vector<unsigned int> get_histogram_from_monochrome_image(const input_array_type *input_array, unsigned int array_size, int output_size)  {
        std::vector<unsigned int> result(output_size, 0);
        for (unsigned int i_pixel = 0; i_pixel < array_size; i_pixel++) {
            result[int(input_array[i_pixel])] += 1;
        }
        return result;
    }

    std::vector<unsigned int> get_histogram_from_rgb_image(const std::vector<std::vector<double>> &image, int output_size);

    // fraction of events with value less than or equal to the given value
    std::vector<double> get_integrated_histogram_from_rgb_image(const std::vector<std::vector<double>> &image, int output_size);

}

