#include "../headers/ImagePreview.h"

#include "../../headers/raw_file_reader.h"
#include "../../headers/ImageFilesInputOutput.h"

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

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



ImagePreview::ImagePreview(int width, int height, int max_value, bool use_color_interpolation)    {
    m_width = width;
    m_height = height;
    m_max_value = max_value;
    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height, 0));
    m_exposure_correction = 0;
    m_use_color_interpolation = use_color_interpolation;
};

void ImagePreview::get_preview_from_stacked_picture(const std::vector<std::vector<double>> &stacked_image,
                                                    int width_original, int height_original,
                                                    int width_resized, int height_resized,
                                                    int *max_value) {

    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(width_resized*height_resized,0)); // 2D vector of brightness values - first index = color, second index = pixel
    std::vector<int>                count(width_resized*height_resized,0);

    const float step_x = width_original / float(width_resized);
    const float step_y =  height_original / float(height_resized);

    m_width_original = width_original;
    m_height_original = height_original;
    m_original_image = std::vector<std::vector<short>>(3, std::vector<short>(width_original*height_original,0));
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_original*height_original; i_pixel++)   {
            m_original_image[i_color][i_pixel] = stacked_image[i_color][i_pixel];
        }
    }


    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_original_y = 0; i_original_y < height_original; i_original_y++)  {
            for (int i_original_x = 0; i_original_x < width_original ; i_original_x++)  {
                const int index = i_original_y * width_original  + i_original_x;
                const int i_pixel_new_y = i_original_y / step_y;
                const int i_pixel_new_x = i_original_x / step_x;
                const int index_new = i_pixel_new_y * width_resized + i_pixel_new_x;
                m_preview_data[i_color][index_new] += m_original_image[i_color][index];
                count[index_new]++;
            }
        }
    }

    *max_value = 0;
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_resized*height_resized; i_pixel++)   {
            if (count[i_pixel] > 0) {
                m_preview_data[i_color][i_pixel] /= count[i_pixel];
            }
            *max_value = max<int>(*max_value, m_preview_data[i_color][i_pixel]);
        }
    }
};

void ImagePreview::read_preview_from_file(const std::string &path)  {
    m_preview_data = get_preview(path, m_width, m_height, &m_max_value);
    m_current_preview_is_raw_file = true;
};

void ImagePreview::read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original)  {
    get_preview_from_stacked_picture(stacked_image, width_original, height_original, m_width, m_height, &m_max_value);
    m_current_preview_is_raw_file = false;
};

void ImagePreview::update_preview_bitmap(wxStaticBitmap *static_bitmap) const  {
    wxImage image_wx(m_width, m_height);

    if (m_current_preview_is_raw_file || m_use_color_interpolation) {
        const float scale_factor = pow(2,m_exposure_correction)*2*255.0 / m_max_value;
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {

                const int index = x + y*m_width;
                image_wx.SetRGB(x, y,   min<int>(255,scale_factor*m_preview_data[0][index]),
                                        min<int>(255,0.5*scale_factor*m_preview_data[1][index]),
                                        min<int>(255,scale_factor*m_preview_data[2][index]));
            }
        }
    }
    else {
        const float scale_factor = pow(2,m_exposure_correction)*255.0 / m_max_value;
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const int index = x + y*m_width;
                image_wx.SetRGB(x, y,   min<int>(255,scale_factor*m_preview_data[0][index]),
                                        min<int>(255,scale_factor*m_preview_data[1][index]),
                                        min<int>(255,scale_factor*m_preview_data[2][index]));
            }
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    static_bitmap->SetBitmap(bitmap);
};

void ImagePreview::zoom_in()    {
    cout << "Zoom in" << endl;
    m_zoom_factor = max<double>(m_zoom_factor*pow(2,1./3), m_max_zoom_factor);
};

void ImagePreview::zoom_out()   {
    cout << "Zoom out" << endl;
    m_zoom_factor = min<double>(m_zoom_factor*pow(2,-1./3), m_min_zoom_factor);
};