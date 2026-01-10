#pragma once

#include <utility>
#include <vector>
#include <stdexcept>

namespace AstroPhotoStacker {
    /**
     * @class LensCorrectionsTool
     * @brief Provides lens barrel correction functionality using radial distortion coefficients.
     */
    class LensCorrectionsTool {
        public:
            LensCorrectionsTool() = delete;

            LensCorrectionsTool(int image_width, int image_height);

            void check_resolution_consistency(int image_width, int image_height) const;

            void set_radial_distortion_coefficients(double k1 = 0, double k2 = 0, double k3 = 0);

            void set_radial_distortion_coefficient_k1(double k1);

            double get_radial_distortion_coefficient_k1() const;

            void set_radial_distortion_coefficient_k2(double k2);

            double get_radial_distortion_coefficient_k2() const;

            void set_radial_distortion_coefficient_k3(double k3);

            double get_radial_distortion_coefficient_k3() const;

            void set_sensor_offset(double offset_in_pixels_x = 0, double offset_in_pixels_y = 0);

            void set_sensor_offset_x(double offset_in_pixels_x);

            void set_sensor_offset_y(double offset_in_pixels_y);

            double get_sensor_offset_x() const ;

            double get_sensor_offset_y() const;

            void initialize();

            int get_corrected_index(int distorted_index) const;

            template<typename PixelDataType>
            void apply_correction_to_data(PixelDataType *data) const {
                if (m_corrected_indices_cache.empty()) {
                    throw std::runtime_error("LensCorrectionsTool: apply_correction_to_data called before initialize()");
                }
                std::vector<PixelDataType> result(m_image_height * m_image_width, -1);
                for (unsigned int distorted_index = 0; distorted_index < m_corrected_indices_cache.size(); distorted_index++) {
                    const int corrected_index = m_corrected_indices_cache[distorted_index];
                    if (corrected_index != -1) {
                        result[corrected_index] = data[distorted_index];
                    }
                }
                std::copy(result.begin(), result.end(), data);
            };


        private:

            double m_k1 = 0.0; // radial distortion coefficient
            double m_k2 = 0.0; // radial distortion coefficient
            double m_k3 = 0.0; // radial distortion coefficient

            double m_sensor_offset_x = 0.0; // sensor offset with respect to the optical axis of the lens
            double m_sensor_offset_y = 0.0; // sensor offset with respect to the optical axis of the lens

            double m_center_x = 0.0; // optical center x coordinate
            double m_center_y = 0.0; // optical center y coordinate

            int m_image_width = 0;
            int m_image_height = 0;
            double m_image_diagonal_in_pixels_squared = 0.0;

            std::vector<int> m_corrected_indices_cache; // index = distorted index, value = corrected index, -1 if out of bounds


            /**
             * @brief Calculate ratio of corrected radius to distorted radius
             *
             * @param r_distorted_squared Squared distance from the optical center in distorted image
             * @return double Ratio of corrected radius to distorted radius
             */
            double calculate_correction_scale_factor(double r_distorted_squared) const;

            double calculate_correction_scale_factor(int x_distorted, int y_distorted) const;

            std::pair<double, double> calculate_corrected_coordinates(int x_distorted, int y_distorted) const;

    };
}