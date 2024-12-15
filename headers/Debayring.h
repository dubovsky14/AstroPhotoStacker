#pragma once

#include <vector>

namespace AstroPhotoStacker {
    template<typename output_type = short int>
    void get_closest_pixel_of_given_color(  const std::vector<output_type> &data_original,
                                            const std::vector<char> &colors_original,
                                            int height,
                                            int width,
                                            int pos_x,
                                            int pos_y,
                                            int color,
                                            int step_x,
                                            int step_y,
                                            short int *value,
                                            int *closest_distance,
                                            int n_steps_max = 2) {

        *closest_distance = -1;
        *value = 0;

        for (int i_step = 1; i_step <= n_steps_max; i_step++)  {
            const int new_pos_x = pos_x + i_step*step_x;
            const int new_pos_y = pos_y + i_step*step_y;
            if (new_pos_x < 0 || new_pos_x >= width || new_pos_y < 0 || new_pos_y >= height) {
                return;
            }
            const int index = new_pos_y*width + new_pos_x;
            if (colors_original[index] == color) {
                *closest_distance = i_step;
                *value = data_original[index];
                return;
            }
        }
    };

    template<typename output_type = short int>
    std::vector<std::vector<output_type>> debayer_raw_data(const std::vector<output_type> &data_original, int width, int height, const std::vector<char> &colors_original)  {
        std::vector<std::vector<output_type>> result;
        for (int color = 0; color < 3; color++) {
            result.push_back(std::vector<output_type>(width*height, 0));
        }

        for (int i_color = 0; i_color < 3; i_color++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    const int index = y*width + x;
                    const int color_original = colors_original[index];
                    result[i_color][index] = data_original[index];

                    short int neighbors_values[8];
                    int neighbors_distances[8];

                    neighbors_distances[0] = (i_color == color_original) ? 1 : -1;
                    neighbors_values[0] = data_original[index];

                    get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 1, 0, &neighbors_values[1], &neighbors_distances[1],1);    // right
                    get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 0, 1, &neighbors_values[2], &neighbors_distances[2],1);    // down
                    get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 1, 1, &neighbors_values[3], &neighbors_distances[3],1);    // right down

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
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 1, 0, &neighbors_values[0], &neighbors_distances[0]);     // right
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, -1, 0, &neighbors_values[1], &neighbors_distances[1]);    // left
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 0, 1, &neighbors_values[2], &neighbors_distances[2]);     // down
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 0, -1, &neighbors_values[3], &neighbors_distances[3]);    // up

                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 1, 1, &neighbors_values[4], &neighbors_distances[4]);     // down-right
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, -1, 1, &neighbors_values[5], &neighbors_distances[5]);    // down-left
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, 1, -1, &neighbors_values[6], &neighbors_distances[6]);    // up-right
                        get_closest_pixel_of_given_color(data_original, colors_original, height, width, x, y, i_color, -1, -1, &neighbors_values[7], &neighbors_distances[7]);   // up-left

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
    }
};
