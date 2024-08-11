#pragma once

#include "../headers/raw_file_reader.h"

#include <vector>
#include <memory>


namespace AstroPhotoStacker {

    /**
     * @class ColorInterpolationTool
     *
     * @brief A class for interpolating colors in a raw image - calculating each color channel for each pixel by averaging the color channels of the neighboring pixels.
    */
    class ColorInterpolationTool {
        public:
            ColorInterpolationTool() = delete;

            /**
             * @brief Constructor that initializes the ColorInterpolationTool object.
             *
             * @param data_original The original image data.
             * @param width The width of the image.
             * @param height The height of the image.
             * @param colors_original The original colors of the image.
             * @param color_conversion_table The color conversion table -> there are usually 2 color codes for green color
            */
            ColorInterpolationTool(const short int *data_original, int width, int height, const std::vector<char> &colors_original, const std::vector<char> &color_conversion_table);

            /**
             * @brief Get the interpolated RGB image.
             *
             * @tparam output_type The data type of the output image.
             * @return The interpolated RGB image as a vector of vectors of the given data type.
            */
            template<typename output_type = short int>
            std::vector<std::vector<output_type>> get_interpolated_rgb_image() const {
                std::vector<std::vector<output_type>> result;
                for (int color = 0; color < 3; color++) {
                    result.push_back(std::vector<output_type>(m_width*m_height, 0));
                }

                for (int i_color = 0; i_color < 3; i_color++) {
                    for (int y = 0; y < m_height; y++) {
                        for (int x = 0; x < m_width; x++) {
                            const int index = y*m_width + x;
                            const int color_original = (*m_color_conversion_table)[(*m_colors_original)[index]]; // there are sometimes 2 color codes for green color ...

                            short int neighbors_values[8];
                            int neighbors_distances[8];

                            neighbors_distances[0] = (i_color == color_original) ? 1 : -1;
                            neighbors_values[0] = m_data_original[index];

                            get_closest_pixel_of_given_color(x, y, i_color, 1, 0, &neighbors_values[1], &neighbors_distances[1],1);    // right
                            get_closest_pixel_of_given_color(x, y, i_color, 0, 1, &neighbors_values[2], &neighbors_distances[2],1);    // down
                            get_closest_pixel_of_given_color(x, y, i_color, 1, 1, &neighbors_values[3], &neighbors_distances[3],1);    // right down

                            float sum = 0;
                            float weight_sum = 0;
                            for (unsigned int i_neighbor = 0; i_neighbor < 4; i_neighbor++) {
                                if (neighbors_distances[i_neighbor] == -1) {
                                    continue;
                                }
                                sum += neighbors_values[i_neighbor];
                                weight_sum += 1;
                            }

                            // this should happen only for rightmost and bottommost pixels
                            if (weight_sum == 0) {
                                get_closest_pixel_of_given_color(x, y, i_color, 1, 0, &neighbors_values[0], &neighbors_distances[0]);     // right
                                get_closest_pixel_of_given_color(x, y, i_color, -1, 0, &neighbors_values[1], &neighbors_distances[1]);    // left
                                get_closest_pixel_of_given_color(x, y, i_color, 0, 1, &neighbors_values[2], &neighbors_distances[2]);     // down
                                get_closest_pixel_of_given_color(x, y, i_color, 0, -1, &neighbors_values[3], &neighbors_distances[3]);    // up

                                get_closest_pixel_of_given_color(x, y, i_color, 1, 1, &neighbors_values[4], &neighbors_distances[4]);     // down-right
                                get_closest_pixel_of_given_color(x, y, i_color, -1, 1, &neighbors_values[5], &neighbors_distances[5]);    // down-left
                                get_closest_pixel_of_given_color(x, y, i_color, 1, -1, &neighbors_values[6], &neighbors_distances[6]);    // up-right
                                get_closest_pixel_of_given_color(x, y, i_color, -1, -1, &neighbors_values[7], &neighbors_distances[7]);   // up-left

                                // now let's calculate weighted mean of the neighbors, with 1/distance as the weight
                                sum = 0;
                                weight_sum = 0;
                                for (unsigned int i_neighbor = 0; i_neighbor < sizeof(neighbors_distances)/sizeof(neighbors_distances[0]); i_neighbor++) {
                                    if (neighbors_distances[i_neighbor] == -1) {
                                        continue;
                                    }
                                    sum += float(neighbors_values[i_neighbor])/neighbors_distances[i_neighbor];
                                    weight_sum += 1.0/neighbors_distances[i_neighbor];
                                }
                            }
                            result[i_color][index] = sum/weight_sum;
                        }
                    }
                }
                return result;
            };

            /**
             * @brief Get the interpolated RGB image from a raw file.
             *
             * @tparam output_type The data type of the output image.
             * @param raw_file The path to the raw file.
             * @param width The width of the image.
             * @param height The height of the image.
             * @return The interpolated RGB image as a vector of vectors of the given data type.
            */
            template<typename output_type = short int>
            static std::vector<std::vector<output_type>> get_interpolated_rgb_image(const std::string &raw_file, int *width, int *height) {
                int temp_width;
                int temp_height;
                std::vector<char> colors;
                std::vector<short> raw_data = read_raw_file<short int>(raw_file, &temp_width, &temp_height, &colors);
                const std::vector<char> color_conversion_table = get_color_info_as_number(raw_file);
                ColorInterpolationTool color_interpolation_tool(raw_data.data(), temp_width, temp_height, colors, color_conversion_table);

                if (width != nullptr) {
                    *width = temp_width;
                }
                if (height != nullptr) {
                    *height = temp_height;
                }
                return color_interpolation_tool.get_interpolated_rgb_image<output_type>();
            };

        private:
            int m_width;
            int m_height;

            const short int *m_data_original = nullptr;
            const std::vector<char> *m_colors_original  = nullptr;
            const std::vector<char> *m_color_conversion_table   = nullptr;


            void get_closest_pixel_of_given_color(  int pos_x,
                                                    int pos_y,
                                                    int color,
                                                    int step_x,
                                                    int step_y,
                                                    short int *value,
                                                    int *closest_distance,
                                                    int n_steps_max = 2) const;
    };
}