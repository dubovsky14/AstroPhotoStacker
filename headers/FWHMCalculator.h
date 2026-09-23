#pragma once

#include "../headers/PixelType.h"

#include <vector>
#include <utility>

namespace AstroPhotoStacker {
    namespace FWHMCalculator {

        /**
         * @brief Calculates the distance from each pixel to the nearest pixel in the cluster.
         *
         * @param pixels_in_cluster A vector of pairs representing the coordinates of pixels in the cluster.
         * @param width The width of the image.
         * @param height The height of the image.
         * @param max_distance The maximum distance to consider.
         * @return A vector of chars representing the distance from each pixel to the nearest pixel in the cluster. Pixels beyond the max_distance will have a value of -1.
         */
        std::vector<char> get_distance_from_cluster_mask(const std::vector<std::pair<int, int>> &pixels_in_cluster, int width, int height,  char max_distance);

        /**
         * @brief Retrieves the pixel values grouped by their distance from the cluster.
         *
         * @param brightness A vector of pixel brightness values.
         * @param distance_mask A vector of chars representing the distance from each pixel to the nearest pixel in the cluster.
         * @return A vector of vectors, where each inner vector contains the pixel values at a specific distance from the cluster.
         */
        std::vector<std::vector<PixelType>> get_pixel_values_in_given_distance_from_cluster(const std::vector<PixelType> &brightness, const std::vector<char> &distance_mask);

        std::vector<std::vector<PixelType>> get_pixel_values_in_given_distance_from_cluster(const std::vector<PixelType> &brightness, int width, int height, const std::vector<std::pair<int, int>> &pixels_in_cluster);

        /**
         * @brief Calculates the Full Width at Half Maximum (FWHM) of the pixel brightness distribution around the cluster (star).
         *
         * @param brightness A vector of pixel brightness values.
         * @param width The width of the image.
         * @param height The height of the image.
         * @param threshold The brightness threshold to determine if the pixel is considered part of the star.
         * @return The calculated FWHM as a float.
         */
        float calculate_fwhm(const std::vector<PixelType> &brightness, int width, int height, PixelType threshold);


        std::vector<double> fit_by_1d_gaussian( const std::vector<double> &data_x,
                                                const std::vector<double> &data_y,
                                                const std::vector<double> &initial_guess,
                                                double learning_rate,
                                                double decay_rate,
                                                double beta,
                                                int max_iterations);

    }
}