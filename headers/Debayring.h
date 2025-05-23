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

        for (int y = 0; y < height-1; y++) {
            for (int x = 0; x < width-1; x++) {
                int this_pixel_rgb[3] = {0, 0, 0};
                int n_pixels[3] = {0, 0, 0};

                // average out 2x2 pixels
                for (int y2 = 0; y2 < 2; y2++)  {
                    for (int x2 = 0; x2 < 2; x2++)   {
                        const unsigned int index = (y+y2)*width + (x+x2);
                        const unsigned int color = colors_original[index];
                        this_pixel_rgb[color] += data_original[index];
                        n_pixels[color]++;
                    }
                }

                // save the result
                for (int color = 0; color < 3; color++) {
                    if (n_pixels[color] == 0)   {
                        result[color][y*width + x] = 0;
                        continue;
                    }
                    result[color][y*width + x] = this_pixel_rgb[color]/n_pixels[color];
                }
            }
        }
        return result;
    }

    template <class ValueType>
    void debayer_monochrome(std::vector<ValueType> *data, int width, int height, const std::vector<char> &colors) {
        for (int y = 0; y < height-1; y++) {
            for (int x = 0; x < width-1; x++) {
                const int index = y*width + x;

                if (colors[index] == 1) { // this one is green, and also the one to bottom right
                    data->at(index) = (static_cast<int>(data->at(index+1)) + static_cast<int>(data->at(index+width)) + data->at(index)/2 + data->at(index+width+1)/2)/3;
                }
                else {
                    data->at(index) = (data->at(index+1)/2 + data->at(index+width)/2 + static_cast<int>(data->at(index)) + static_cast<int>(data->at(index+width+1)))/3;
                }
            }
        }
    };
};
