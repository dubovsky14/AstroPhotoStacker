#pragma once

#include "../headers/LightPollutionGradientFunctions.h"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace AstroPhotoStacker {

    std::unique_ptr<LightPollutionGradientBase> fit_gradient(const std::vector<std::pair<double, double>> &coordinates, const std::vector<double> &integrated_value, const std::string &function_type = "polynomial2n");

    template<typename T>
    std::vector<T> subtract_gradient(const std::vector<T> &input_image, const unsigned int width, const unsigned int height, const LightPollutionGradientBase &gradient_function) {
        std::vector<T> output_image(input_image.size(), 0.0);
        for (unsigned int y = 0; y < height; y++) {
            for (unsigned int x = 0; x < width; x++) {
                const double gradient_value = gradient_function.get_value(x, y);
                double new_value = input_image[y * width + x] - gradient_value;
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
                                                const std::vector<std::pair<double, double>> &coordinates_top_left,
                                                const std::vector<std::pair<double, double>> &coordinates_bottom_right)    {

        if (coordinates_top_left.size() != coordinates_bottom_right.size()) {
            throw std::invalid_argument("Top left and bottom right coordinate vectors must be of the same size");
        }

        std::vector<double> integrated_values(coordinates_top_left.size(), 0.0);
        for (unsigned int i_sample = 0; i_sample < coordinates_top_left.size(); i_sample++) {
            if (coordinates_top_left[i_sample].first >= coordinates_bottom_right[i_sample].first ||
                coordinates_top_left[i_sample].second >= coordinates_bottom_right[i_sample].second) {
                throw std::invalid_argument("Top left coordinates must be less than bottom right coordinates for each sample");
            }

            double sum = 0.0;
            std::vector<double> sample_values;
            sample_values.reserve((coordinates_bottom_right[i_sample].first - coordinates_top_left[i_sample].first) * (coordinates_bottom_right[i_sample].second - coordinates_top_left[i_sample].second));
            for (unsigned int y = coordinates_top_left[i_sample].second; y < coordinates_bottom_right[i_sample].second; y++) {
                for (unsigned int x = coordinates_top_left[i_sample].first; x < coordinates_bottom_right[i_sample].first; x++) {
                    sample_values.push_back(input_image[y * input_image_width + x]);
                }
            }
            std::sort(sample_values.begin(), sample_values.end());
            const double median = sample_values[sample_values.size() / 2];
            integrated_values[i_sample] = median;

        }
        return integrated_values;
    };


}