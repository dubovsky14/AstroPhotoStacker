#pragma once

#include "../headers/PixelType.h"

#include <vector>
#include <utility>
#include <array>
#include <stdexcept>
#include <tuple>

namespace AstroPhotoStacker {


    /**
     * @brief A view into a 2D image, allowing access to a subregion of the original image without copying the data.
     */
    template<typename PixelValueType>
    class ImageView2D {
        public:
            ImageView2D(const PixelValueType *data, size_t original_width, size_t original_height, size_t top_left_x, size_t top_left_y, size_t view_width, size_t view_height) {
                m_original_width = original_width;
                m_original_height = original_height;
                m_top_left_x = top_left_x;
                m_top_left_y = top_left_y;
                m_view_width = view_width;
                m_view_height = view_height;
                m_data = data;

                if (m_top_left_x + m_view_width > m_original_width || m_top_left_y + m_view_height > m_original_height) {
                    throw std::out_of_range("View exceeds original image bounds");
                }
            }

            const PixelValueType at(int x, int y) const {
                if (x < 0 || x >= m_view_width || y < 0 || y >= m_view_height) {
                    throw std::out_of_range("Index out of range");
                }

                const size_t original_x = m_top_left_x + x;
                const size_t original_y = m_top_left_y + y;

                return m_data[original_y * m_original_width + original_x];
            }

            size_t get_view_width() const { return m_view_width; };
            size_t get_view_height() const { return m_view_height; };

            size_t get_original_width() const { return m_original_width; };
            size_t get_original_height() const { return m_original_height; };

        private:
            size_t m_view_width;
            size_t m_view_height;
            size_t m_top_left_x;
            size_t m_top_left_y;

            size_t m_original_width;
            size_t m_original_height;

            const PixelValueType *m_data;
    };



    namespace FWHMCalculator {

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

        float get_background_median(const ImageView2D<PixelType> &image_view, int edge_margin = 2);

        // Returns the bounding box of the cluster as {min_x, min_y, max_x, max_y}
        std::array<int, 4> get_cluster_bounds(const std::vector<std::tuple<int,int>> &cluster);

        std::pair<std::vector<PixelType>, std::vector<PixelType>> get_line_above_and_below(
            const ImageView2D<PixelType> &image_view,
            const std::vector<std::tuple<int,int>> &cluster,
            int cluster_y_min,
            int cluster_y_max,
            float background_median,
            int edge_margin = 20
        );

        /**
         * @brief Fits a Full Width at Half Maximum (FWHM) model to the provided data using polynomial approximation.
         *
         * @param x_data The x-coordinates of the data points.
         * @param y_data The y-coordinates of the data points.
         */
        class FWHMFitter {
            public:
                FWHMFitter() = delete;

                FWHMFitter(const std::vector<float> &x_data, const std::vector<float> &y_data);

                //float get_fwhm() const;

                float operator()(float x, int segment_index) const;

                // [x,y] coordinates of the maximum point within the specified segment.
                std::pair<float, float> get_segment_maximum(int segment_index) const;

                //float get_amplitude() const;

            private:
                std::vector<float> m_x_data;
                std::vector<float> m_y_data;

                std::vector<std::pair<float, float>> m_slopes_and_offsets;
        };


        std::array<double, 3> fit_by_1d_gaussian( const std::vector<double> &data_x,
                                                const std::vector<double> &data_y,
                                                const std::array<double, 3> &initial_guess,
                                                double learning_rate,
                                                double decay_rate,
                                                double beta,
                                                int max_iterations);

    }
}