#include "../headers/ImagePreview.h"

#include "../../headers/raw_file_reader.h"
#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/ColorInterpolationTool.h"

#include <opencv2/opencv.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

ImagePreview::ImagePreview(int width, int height, int max_value, bool use_color_interpolation)    {
    m_width = width;
    m_height = height;
    m_max_value = max_value;
    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height, 0));
    m_exposure_correction = 0;
    m_use_color_interpolation = use_color_interpolation;
};

void ImagePreview::read_preview_from_file(const std::string &path)  {
    m_current_preview_is_raw_file = is_raw_file(path);
    if (m_current_preview_is_raw_file)  {
        m_original_image = ColorInterpolationTool::get_interpolated_rgb_image(path, &m_width_original, &m_height_original);
    }
    else {
        // picture file
        cv::Mat image = cv::imread(path, 1);
        m_width_original = image.cols;
        m_height_original = image.rows;
        m_original_image = std::vector<std::vector<unsigned short int>>(3, std::vector<unsigned short int>(m_width_original*m_height_original,0));
        for (int y = 0; y < m_height_original; y++) {
            for (int x = 0; x < m_width_original; x++) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(y,x);
                m_original_image[0][y*m_width_original + x] = pixel[2];
                m_original_image[1][y*m_width_original + x] = 2*pixel[1];
                m_original_image[2][y*m_width_original + x] = pixel[0];
            }
        }
    }
    set_default_resized_area();
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original)  {
    m_current_preview_is_raw_file = false;
    m_width_original = width_original;
    m_height_original = height_original;
    m_original_image = std::vector<std::vector<unsigned short int>>(3, std::vector<unsigned short int>(width_original*height_original,0));
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_original*height_original; i_pixel++)   {
            m_original_image[i_color][i_pixel] = stacked_image[i_color][i_pixel];
        }
    }
    set_default_resized_area();
    m_current_preview_is_raw_file = false;
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::update_preview_bitmap(wxStaticBitmap *static_bitmap) const  {
    const bool apply_green_correction = (m_current_preview_is_raw_file || m_use_color_interpolation);
    update_preview_bitmap(static_bitmap, apply_green_correction);
};

void ImagePreview::update_preview_bitmap(wxStaticBitmap *static_bitmap, bool apply_green_correction) const  {
    wxImage image_wx(m_width, m_height);
    auto set_pixels = [&image_wx, this](float green_channel_correction) {
        const float scale_factor = pow(2,m_exposure_correction)*2*255.0 / m_max_value;
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const int index = x + y*m_width;
                int red   = m_preview_data[0][index];
                int green = m_preview_data[1][index];
                int blue  = m_preview_data[2][index];

                if (m_color_stretcher != nullptr) {
                    if (m_color_stretcher->has_stretchers()) {
                        for (int i_color = 0; i_color < 3; i_color++) {
                            red   = m_color_stretcher->stretch(red,   m_max_value, 0);
                            green = m_color_stretcher->stretch(green, m_max_value, 1);
                            blue  = m_color_stretcher->stretch(blue,  m_max_value, 2);
                        }
                    }
                }
                red   = min<int>(255,scale_factor*red  );
                green = min<int>(255,scale_factor*green*green_channel_correction);
                blue  = min<int>(255,scale_factor*blue );
                image_wx.SetRGB(x, y, red, green, blue);
            }
        }
    };

    if (apply_green_correction) {
        set_pixels(0.5);
    }
    else {
        set_pixels(1);
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    static_bitmap->SetBitmap(bitmap);
};

void ImagePreview::zoom_in(float mouse_position_relative_x, float mouse_position_relative_y)    {
    m_zoom_factor = min<double>(m_zoom_factor*pow(2,1./3), m_max_zoom_factor);
    update_preview_data(mouse_position_relative_x, mouse_position_relative_y);
};

void ImagePreview::zoom_out(float mouse_position_relative_x, float mouse_position_relative_y)   {
    m_zoom_factor = max<double>(m_zoom_factor*pow(2,-1./3), m_min_zoom_factor);
    update_preview_data(mouse_position_relative_x, mouse_position_relative_y);
};

void ImagePreview::set_default_resized_area()   {
    if (m_i_x_resized_max < 0)  {
        m_i_x_resized_min = 0;
        m_i_x_resized_max = m_width_original;
        m_i_y_resized_min = 0;
        m_i_y_resized_max = m_height_original;
    }
};

void ImagePreview::set_stretcher(const CombinedColorStrecherTool *color_stretcher)    {
    m_color_stretcher = color_stretcher;
};

void ImagePreview::update_max_values_original()    {
    m_max_values_original = std::vector<int>(3,0);
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < m_width_original*m_height_original; i_pixel++)   {
            m_max_values_original[i_color] = max<int>(m_max_values_original[i_color], m_original_image[i_color][i_pixel]);
        }
    }
    m_max_value = *max_element(m_max_values_original.begin(), m_max_values_original.end());
};

void ImagePreview::update_preview_data(float mouse_position_relative_x, float mouse_position_relative_y)    {

    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height,0)); // 2D vector of brightness values - first index = color, second index = pixel
    std::vector<int>                count(m_width*m_height,0);

    const float step_x =  m_width_original  / (m_zoom_factor*float(m_width) );
    const float step_y =  m_height_original / (m_zoom_factor*float(m_height));

    int x_center = m_i_x_resized_min + (m_i_x_resized_max - m_i_x_resized_min)*mouse_position_relative_x;
    if (x_center < m_width_original/(m_zoom_factor*2)) {
        x_center = m_width_original/(m_zoom_factor*2);
    }
    if (x_center > m_width_original - m_width_original/(m_zoom_factor*2)) {
        x_center = m_width_original - m_width_original/(m_zoom_factor*2);
    }

    int y_center = m_i_y_resized_min + (m_i_y_resized_max - m_i_y_resized_min)*mouse_position_relative_y;
    if (y_center < m_height_original/(m_zoom_factor*2)) {
        y_center = m_height_original/(m_zoom_factor*2);
    }
    if (y_center > m_height_original - m_height_original/(m_zoom_factor*2)) {
        y_center = m_height_original - m_height_original/(m_zoom_factor*2);
    }

    m_i_x_resized_min = max<int>(0, x_center - m_width_original/(m_zoom_factor*2));
    m_i_x_resized_max = min<int>(m_width_original, x_center + m_width_original/(m_zoom_factor*2));

    m_i_y_resized_min = max<int>(0, y_center - m_height_original/(m_zoom_factor*2));
    m_i_y_resized_max = min<int>(m_height_original, y_center + m_height_original/(m_zoom_factor*2));


    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_original_y = m_i_y_resized_min; i_original_y < m_i_y_resized_max; i_original_y++)  {
            for (int i_original_x = m_i_x_resized_min; i_original_x < m_i_x_resized_max ; i_original_x++)  {
                const int index = i_original_y * m_width_original  + i_original_x;
                const int i_pixel_new_y = (i_original_y-m_i_y_resized_min) / step_y;
                const int i_pixel_new_x = (i_original_x-m_i_x_resized_min) / step_x;
                const int index_new = i_pixel_new_y * m_width + i_pixel_new_x;
                m_preview_data[i_color][index_new] += m_original_image[i_color][index];
                count.at(index_new)++;
            }
        }
    }

    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++)   {
            if (count[i_pixel] > 0) {
                m_preview_data[i_color][i_pixel] /= count[i_pixel];
            }
        }
    }
};