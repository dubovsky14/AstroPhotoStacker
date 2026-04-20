#pragma once

#include "../headers/LightPollutionGradientFunctions.h"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace AstroPhotoStacker {

    struct SampleWindow {
        std::pair<double, double> top_left;
        std::pair<double, double> bottom_right;
    };

    std::unique_ptr<LightPollutionGradientBase> fit_gradient(   const std::vector<std::pair<double, double>> &coordinates,
                                                                int width, int height,
                                                                const std::vector<double> &integrated_value,
                                                                const std::string &function_type = "polynomial2n");

    template<typename T>
    std::vector<T> subtract_gradient(const std::vector<T> &input_image, const unsigned int width, const unsigned int height, const LightPollutionGradientBase &gradient_function, bool ignore_offsett = true)    {
        std::vector<T> output_image(input_image.size(), 0.0);
        const double offset = ignore_offsett ? gradient_function.get_parameters()[0] : 0.0;
        for (unsigned int y = 0; y < height; y++) {
            for (unsigned int x = 0; x < width; x++) {
                const double gradient_value = gradient_function.get_value(x, y);
                double new_value = input_image[y * width + x] - gradient_value + offset;
                if (new_value < 0) {
                    new_value = 0; // Ensure no negative pixel values
                }
                output_image[y * width + x] = static_cast<T>(new_value);
            }
        }
        return output_image;
    };

    template<typename T>
    std::vector<double> integrate_each_sample(  const std::vector<T> &input_image,
                                                const unsigned int input_image_width,
                                                const std::vector<SampleWindow> &sample_windows)    {

        std::vector<double> integrated_values(sample_windows.size(), 0.0);
        for (unsigned int i_sample = 0; i_sample < sample_windows.size(); i_sample++) {
            if (sample_windows[i_sample].top_left.first >= sample_windows[i_sample].bottom_right.first ||
                sample_windows[i_sample].top_left.second >= sample_windows[i_sample].bottom_right.second) {
                throw std::invalid_argument("Top left coordinates must be less than bottom right coordinates for each sample");
            }

            std::vector<double> sample_values;
            sample_values.reserve((sample_windows[i_sample].bottom_right.first - sample_windows[i_sample].top_left.first) * (sample_windows[i_sample].bottom_right.second - sample_windows[i_sample].top_left.second));
            for (unsigned int y = sample_windows[i_sample].top_left.second; y < sample_windows[i_sample].bottom_right.second; y++) {
                for (unsigned int x = sample_windows[i_sample].top_left.first; x < sample_windows[i_sample].bottom_right.first; x++) {
                    sample_values.push_back(input_image[y * input_image_width + x]);
                }
            }
            std::sort(sample_values.begin(), sample_values.end());
            const double median = sample_values[sample_values.size() / 4];
            integrated_values[i_sample] = median;

        }
        return integrated_values;
    };


}