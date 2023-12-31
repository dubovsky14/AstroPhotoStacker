#include "../headers/ImagePreview.h"

#include "../../headers/raw_file_reader.h"
#include "../../headers/ImageFilesInputOutput.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

std::vector<std::vector<int>> get_preview(const std::string &path, int width, int height, int *max_value)   {
    std::vector<std::vector<int>> preview(3, std::vector<int>(width*height,0)); // 2D vector of brightness values - first index = color, second index = pixel

    int raw_width, raw_height;
    std::vector<char> colors;
    std::unique_ptr<unsigned short[]> brightness = nullptr;
    try {
        brightness = read_raw_file<unsigned short>(path, &raw_width, &raw_height, &colors);
    }
    catch (std::exception &e)   {
        return preview;
    }

    const float step_x = raw_width / float(width);
    const float step_y =  raw_height / float(height);

    std::vector<std::vector<int>> count(3, std::vector<int>(width*height,0));
    for (int i_original_y = 0; i_original_y < raw_height; i_original_y++)  {
        for (int i_original_x = 0; i_original_x < raw_width; i_original_x++)  {
            const int index = i_original_y * raw_width + i_original_x;
            int color = colors[index];
            if (color == 3) color = 1;
            const int i_pixel_new_y = i_original_y / step_y;
            const int i_pixel_new_x = i_original_x / step_x;
            const int index_new = i_pixel_new_y * width + i_pixel_new_x;
            count  [color][index_new]++;
            preview[color][index_new] += brightness[index];
        }
    }

    *max_value = 0;
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width*height; i_pixel++)   {
            if (count[i_color][i_pixel] > 0) {
                preview[i_color][i_pixel] /= count[i_color][i_pixel];
            }
            *max_value = max<int>(*max_value, preview[i_color][i_pixel]);
        }
    }

    return preview;
};

std::vector<std::vector<int>> get_preview_from_stacked_picture(const std::vector<std::vector<double>> stacked_image,
                                                                int width_original, int height_original,
                                                                int width_resized, int height_resized,
                                                                int *max_value) {

    std::vector<std::vector<int>>   preview(3, std::vector<int>(width_resized*height_resized,0)); // 2D vector of brightness values - first index = color, second index = pixel
    std::vector<int>                count(width_resized*height_resized,0);

    const float step_x = width_original / float(width_resized);
    const float step_y =  height_original / float(height_resized);


    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_original_y = 0; i_original_y < height_original; i_original_y++)  {
            for (int i_original_x = 0; i_original_x < width_original ; i_original_x++)  {
                const int index = i_original_y * width_original  + i_original_x;
                const int i_pixel_new_y = i_original_y / step_y;
                const int i_pixel_new_x = i_original_x / step_x;
                const int index_new = i_pixel_new_y * width_resized + i_pixel_new_x;
                preview[i_color][index_new] += stacked_image[i_color][index];
                count[index_new]++;
            }
        }
    }

    *max_value = 0;
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_resized*height_resized; i_pixel++)   {
            if (count[i_pixel] > 0) {
                preview[i_color][i_pixel] /= count[i_pixel];
            }
            *max_value = max<int>(*max_value, preview[i_color][i_pixel]);
        }
    }

    return preview;

};
