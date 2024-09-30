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

ImagePreview::ImagePreview(wxFrame *parent, int width, int height, int max_value, bool use_color_interpolation)    {
    m_parent = parent;
    m_width = width;
    m_height = height;
    m_image_resize_tool.set_preview_size(m_width, m_height);
    m_image_resize_tool.set_zoom_factor(1);
    m_max_value = max_value;
    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height, 0));
    m_exposure_correction = 0;
    m_use_color_interpolation = use_color_interpolation;
    initialize_bitmap();
};

void ImagePreview::initialize_bitmap()    {
    // Create a wxImage
    wxImage image(m_width, m_height);

    // Set the image to black
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            image.SetRGB(x, y, 0,0,0);
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Create a wxStaticBitmap to display the image
    m_preview_bitmap = new wxStaticBitmap(m_parent, wxID_ANY, bitmap);

    m_parent->Bind(wxEVT_MOUSEWHEEL, &ImagePreview::on_mouse_wheel, this);
};

void ImagePreview::read_preview_from_file(const std::string &path)  {
    m_current_preview_is_raw_file = is_raw_file(path);
    if (m_current_preview_is_raw_file)  {
        int width_original, height_original;
        m_original_image = ColorInterpolationTool::get_interpolated_rgb_image(path, &width_original, &height_original);
        m_image_resize_tool.set_original_size(width_original, height_original);
    }
    else {
        // picture file
        cv::Mat image = cv::imread(path, 1);
        m_image_resize_tool.set_original_size(image.cols, image.rows);
        m_original_image = std::vector<std::vector<short int>>(3, std::vector<short int>(
            m_image_resize_tool.get_width_original()* m_image_resize_tool.get_height_original(),0)
        );

        for (int y = 0; y < m_image_resize_tool.get_height_original(); y++) {
            for (int x = 0; x < m_image_resize_tool.get_width_original(); x++) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(y,x);
                m_original_image[0][y*m_image_resize_tool.get_width_original() + x] = pixel[2];
                m_original_image[1][y*m_image_resize_tool.get_width_original() + x] = 2*pixel[1];
                m_original_image[2][y*m_image_resize_tool.get_width_original() + x] = pixel[0];
            }
        }
    }
    m_image_resize_tool.set_default_resized_area();
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original)  {
    m_current_preview_is_raw_file = false;
    m_image_resize_tool.set_original_size(width_original, height_original);
    m_original_image = std::vector<std::vector<short int>>(3, std::vector<short int>(width_original*height_original,0));
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_original*height_original; i_pixel++)   {
            m_original_image[i_color][i_pixel] = stacked_image[i_color][i_pixel];
        }
    }
    m_image_resize_tool.set_default_resized_area();
    m_current_preview_is_raw_file = false;
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::update_preview_bitmap()  const  {
    const bool apply_green_correction = (m_current_preview_is_raw_file || m_use_color_interpolation);
    update_preview_bitmap(apply_green_correction);
};

wxImage ImagePreview::get_updated_wximage(bool apply_green_correction)  const  {
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
                        red   = m_color_stretcher->stretch(red,   m_max_value, 0);
                        green = m_color_stretcher->stretch(green, m_max_value, 1);
                        blue  = m_color_stretcher->stretch(blue,  m_max_value, 2);
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
    return image_wx;
};

void ImagePreview::update_preview_bitmap(bool apply_green_correction)   const  {
    wxImage image_wx = get_updated_wximage(apply_green_correction);

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};

void ImagePreview::zoom_in(float mouse_position_relative_x, float mouse_position_relative_y)    {
    m_zoom_factor = min<double>(m_zoom_factor*pow(2,1./3), m_max_zoom_factor);
    m_image_resize_tool.set_zoom_factor(m_zoom_factor);
    update_preview_data(mouse_position_relative_x, mouse_position_relative_y);
};

void ImagePreview::zoom_out(float mouse_position_relative_x, float mouse_position_relative_y)   {
    m_zoom_factor = max<double>(m_zoom_factor*pow(2,-1./3), m_min_zoom_factor);
    m_image_resize_tool.set_zoom_factor(m_zoom_factor);
    update_preview_data(mouse_position_relative_x, mouse_position_relative_y);
};



void ImagePreview::set_stretcher(const CombinedColorStrecherTool *color_stretcher)    {
    m_color_stretcher = color_stretcher;
};

void ImagePreview::update_max_values_original()    {
    m_max_values_original = std::vector<int>(3,0);
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < m_image_resize_tool.get_width_original()*m_image_resize_tool.get_height_original(); i_pixel++)   {
            m_max_values_original[i_color] = max<int>(m_max_values_original[i_color], m_original_image[i_color][i_pixel]);
        }
    }
    m_max_value = *max_element(m_max_values_original.begin(), m_max_values_original.end());
};

void ImagePreview::update_preview_data(float mouse_position_relative_x, float mouse_position_relative_y)    {

    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height,0)); // 2D vector of brightness values - first index = color, second index = pixel
    std::vector<int>                count(m_width*m_height,0);

    m_image_resize_tool.set_relative_mouse_position(mouse_position_relative_x, mouse_position_relative_y);
    m_image_resize_tool.update();

    int x_resized_min, x_resized_max, y_resized_min, y_resized_max;
    m_image_resize_tool.get_crop_borders_in_original_coordinates(&x_resized_min, &x_resized_max, &y_resized_min, &y_resized_max);

    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_original_y = y_resized_min; i_original_y < y_resized_max; i_original_y++)  {
            for (int i_original_x = x_resized_min; i_original_x < x_resized_max ; i_original_x++)  {
                const int index = i_original_y * m_image_resize_tool.get_width_original()  + i_original_x;

                const int pixel_preview_x = m_image_resize_tool.get_preview_coordinate_x(i_original_x);
                const int pixel_preview_y = m_image_resize_tool.get_preview_coordinate_y(i_original_y);

                const int index_new = pixel_preview_y * m_width + pixel_preview_x;
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

void ImagePreview::on_mouse_wheel(wxMouseEvent& event) {
    if (m_original_image.size() == 0) {
        return;
    }
    // Get the mouse position in screen coordinates
    wxPoint screen_pos = event.GetPosition();
    screen_pos += wxPoint(0, 0.155*m_height);   // shift the position to the center of the image - wxStaticBitmap is buggy ...

    // Convert the mouse position to client coordinates relative to the wxStaticBitmap
    wxPoint client_position = m_preview_bitmap->ScreenToClient(screen_pos);

    // Check if the mouse is over the wxStaticBitmap
    if (wxRect(m_preview_bitmap->GetSize()).Contains(client_position)) {
        // Get the amount of rotation
        int rotation = event.GetWheelRotation();

        // Calculate the relative position of the mouse within the wxStaticBitmap
        wxSize bitmapSize = m_preview_bitmap->GetSize();
        float relative_x = static_cast<float>(client_position.x) / bitmapSize.GetWidth();
        float relative_y = static_cast<float>(client_position.y) / bitmapSize.GetHeight();

        // Check the direction of the rotation
        if (rotation > 0) {
            this->zoom_in(relative_x, relative_y);
            this->update_preview_bitmap();
        } else if (rotation < 0) {
            this->zoom_out(relative_x, relative_y);
            this->update_preview_bitmap();
        }
    }
};