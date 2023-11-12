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


    cout << "Preview file loaded" << endl;
    cout << "R channel size: " << preview[0].size() << endl;
    cout << "G channel size: " << preview[1].size() << endl;
    cout << "B channel size: " << preview[2].size() << endl;

    cout << "Going to save preview file" << endl;
    crate_color_image(preview[0].data(), preview[1].data(), preview[2].data(), width, height, "/home/dubovsky/Documents/zdrojaky/AstroPhotoStacker/GUI/bin/preview.tif", CV_16UC3);
    //create_gray_scale_image(preview[0].data(), width, height, "/home/dubovsky/Documents/zdrojaky/AstroPhotoStacker/GUI/bin/preview_red.tif", CV_16UC1);

    cout << "Preview file saved" << endl;

    return preview;
};


/*
    for (int i_pixel_new_y = 0; i_pixel_new_y < height; i_pixel_new_y++)   {
        for (int i_pixel_new_x = 0; i_pixel_new_x < width; i_pixel_new_x++)   {
            int count[] = {0, 0, 0};
            int sum[] = {0, 0, 0};

            // now calculate sum over original pixels in individual RGB channels
            const int y_original_min = i_pixel_new_y * step_y;
            const int y_original_max = max<int>(((i_pixel_new_y + 1) * step_y),raw_height-1);
            const int x_original_min = i_pixel_new_x * step_x;
            const int x_original_max = max<int>(((i_pixel_new_x + 1) * step_x),raw_width-1);

            for (int y_original = y_original_min; y_original < y_original_max; ++y_original)   {
                for (int x_original = x_original_min; x_original < x_original_max; ++x_original)   {
                    const int index = y_original * raw_width + x_original;
                    int color = colors[index];
                    if (color == 3) color = 1;
                    count[color]++;
                    sum[color] += brightness[index];
                }
            }

            const int index_new = i_pixel_new_y * width + i_pixel_new_x;

            // now calculate average
            for (int i_color = 0; i_color < 3; ++i_color)   {
                if (count[i_color] > 0) {
                    preview[i_color][index_new] = sum[i_color] / count[i_color];
                }
                else {
                    preview[i_color][index_new] = 0;
                }
            }
        }
    }
*/