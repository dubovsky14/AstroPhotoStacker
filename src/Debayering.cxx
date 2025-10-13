#include "../headers/Debayring.h"

using namespace AstroPhotoStacker;
using namespace std;

void AstroPhotoStacker::get_closest_pixel_of_given_color(  const std::vector<PixelType> &data_original,
                                        const std::array<char, 4> &bayer_pattern,
                                        int height,
                                        int width,
                                        int pos_x,
                                        int pos_y,
                                        int color,
                                        int step_x,
                                        int step_y,
                                        PixelType *value,
                                        int *closest_distance,
                                        int n_steps_max) {

    *closest_distance = -1;
    *value = 0;

    for (int i_step = 1; i_step <= n_steps_max; i_step++)  {
        const int new_pos_x = pos_x + i_step*step_x;
        const int new_pos_y = pos_y + i_step*step_y;
        if (new_pos_x < 0 || new_pos_x >= width || new_pos_y < 0 || new_pos_y >= height) {
            return;
        }
        const int index = new_pos_y*width + new_pos_x;
        const int bayer_index = new_pos_y%2*2 + new_pos_x%2;
        if (bayer_pattern[bayer_index] == color) {
            *closest_distance = i_step;
            *value = data_original[index];
            return;
        }
    }
};

std::vector<std::vector<PixelType>> AstroPhotoStacker::debayer_raw_data(const std::vector<PixelType> &data_original, int width, int height, const std::array<char, 4> &bayer_pattern)  {
    std::vector<std::vector<PixelType>> result;
    for (int color = 0; color < 3; color++) {
        result.push_back(std::vector<PixelType>(width*height, 0));
    }

    for (int y = 0; y < height-1; y++) {
        for (int x = 0; x < width-1; x++) {
            int this_pixel_rgb[3] = {0, 0, 0};
            int n_pixels[3] = {0, 0, 0};

            // average out 2x2 pixels
            for (int y2 = 0; y2 < 2; y2++)  {
                for (int x2 = 0; x2 < 2; x2++)   {
                    const unsigned int index = (y+y2)*width + (x+x2);
                    const int bayer_index = ((y2+y)%2)*2 + (x+x2)%2;
                    const unsigned int color = bayer_pattern[bayer_index];
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

void AstroPhotoStacker::debayer_monochrome(std::vector<PixelType> *data, int width, int height, const std::array<char, 4> &bayer_pattern) {
    for (int y = 0; y < height-1; y++) {
        for (int x = 0; x < width-1; x++) {
            const int index = y*width + x;
            const int bayer_index = (y%2)*2 + (x%2);
            if (bayer_pattern[bayer_index] == 1) { // this one is green, and also the one to bottom right
                data->at(index) = (static_cast<int>(data->at(index+1)) + static_cast<int>(data->at(index+width)) + data->at(index)/2 + data->at(index+width+1)/2)/3;
            }
            else {
                data->at(index) = (data->at(index+1)/2 + data->at(index+width)/2 + static_cast<int>(data->at(index)) + static_cast<int>(data->at(index+width+1)))/3;
            }
        }
    }
};

