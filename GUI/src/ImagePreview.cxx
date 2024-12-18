#include "../headers/ImagePreview.h"

#include "../../headers/InputFrameData.h"


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
    initialize_bitmap();
    bind_shift_events();
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

    // Create a wxGenericStaticBitmap to display the image
    m_preview_bitmap = new wxGenericStaticBitmap(m_parent, wxID_ANY, bitmap);

    m_parent->Bind(wxEVT_MOUSEWHEEL, &ImagePreview::on_mouse_wheel, this);
};

void ImagePreview::read_preview_from_frame(const InputFrame &input_frame)  {
    InputFrameData<short int> input_frame_data(input_frame);
    if (input_frame_data.is_raw_file()) {
        input_frame_data.debayer();
    }
    m_original_image = input_frame_data.get_image_data_color();
    const int width_original = input_frame_data.get_width();
    const int height_original = input_frame_data.get_height();
    m_image_resize_tool.set_original_size(width_original, height_original);

    m_max_zoom_factor = std::min<double>(m_image_resize_tool.get_height_original()/m_height, m_image_resize_tool.get_width_original()/m_width);
    m_image_resize_tool.set_default_resized_area();
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::read_preview_from_file(const std::string &path)  {
    read_preview_from_frame(InputFrame(path));
};

void ImagePreview::update_original_image(const std::vector<std::vector<short int>> &original_image, int width, int height)   {
    m_image_resize_tool.set_original_size(width, height);
    m_original_image = original_image;
    m_image_resize_tool.set_default_resized_area();
    update_max_values_original();
    update_preview_data();
};

void ImagePreview::read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original)  {
    std::vector<std::vector<short int>> stacked_image_short_int(3, std::vector<short int>(width_original*height_original,0));
    for (int i_color = 0; i_color < 3; i_color++)   {
        for (int i_pixel = 0; i_pixel < width_original*height_original; i_pixel++)   {
            stacked_image_short_int[i_color][i_pixel] = stacked_image[i_color][i_pixel];
        }
    }
    update_original_image(stacked_image_short_int, width_original, height_original);
};

wxImage ImagePreview::get_updated_wximage()  const  {
    wxImage image_wx(m_width, m_height);
    const bool apply_additional_layers = m_additional_layers_preview.size() == m_original_image.size();

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
            green = min<int>(255,scale_factor*green);
            blue  = min<int>(255,scale_factor*blue );

            if (apply_additional_layers) {
                const bool valid_pixel = m_additional_layers_preview[0][index] >= 0;
                if (valid_pixel) {
                    red   = m_additional_layers_preview[0][index];
                    green = m_additional_layers_preview[1][index];
                    blue  = m_additional_layers_preview[2][index];
                }
            }
            image_wx.SetRGB(x, y, red, green, blue);
        }
    }

    return image_wx;
};

void ImagePreview::update_preview_bitmap()   {
    wxImage image_wx = get_updated_wximage();

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};

void ImagePreview::zoom(float mouse_position_relative_x, float mouse_position_relative_y, float zoom_step)  {
    const double old_zoom_factor = m_zoom_factor;
    m_zoom_factor = force_range<double>(m_zoom_factor*pow(2,zoom_step), 1, m_max_zoom_factor);
    m_image_resize_tool.set_zoom_factor(m_zoom_factor);

    const double zoom_factor_ratio = old_zoom_factor/m_zoom_factor;
    if (zoom_factor_ratio == 1) {
        return;
    }

    // make sure the mouse position stays in the same place
    const double mouse_position_relative_x_new = force_range<double>(0.5+(mouse_position_relative_x-0.5)*(1-zoom_factor_ratio), 0, 1);
    const double mouse_position_relative_y_new = force_range<double>(0.5+(mouse_position_relative_y-0.5)*(1-zoom_factor_ratio), 0, 1);

    update_preview_data(mouse_position_relative_x_new, mouse_position_relative_y_new);
};

void ImagePreview::zoom_in(float mouse_position_relative_x, float mouse_position_relative_y)    {
    zoom(mouse_position_relative_x, mouse_position_relative_y, 1./3);
};

void ImagePreview::zoom_out(float mouse_position_relative_x, float mouse_position_relative_y)   {
    zoom(mouse_position_relative_x, mouse_position_relative_y, -1./3);
};

void ImagePreview::set_stretcher(const CombinedColorStrecherTool *color_stretcher)    {
    m_color_stretcher = color_stretcher;
};

const std::vector<std::vector<short int>>& ImagePreview::get_original_image(int *width, int *height) const    {
    if (width != nullptr) {
        *width = m_image_resize_tool.get_width_original();
    }
    if (height != nullptr) {
        *height = m_image_resize_tool.get_height_original();
    }
    return m_original_image;
};

void ImagePreview::add_layer(const std::string &layer_name, const std::function<void(std::vector<std::vector<short int>> *, int, int)> &functor)   {
    m_additional_layers_functors[layer_name] = functor;
    update_additional_layers_data();
};

void ImagePreview::remove_layer(const std::string &layer_name)    {
    if (m_additional_layers_functors.find(layer_name) == m_additional_layers_functors.end()) {
        return;
    }
    m_additional_layers_functors.erase(layer_name);
    update_additional_layers_data();
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
    const bool has_additional_layers = m_additional_layers_functors.size() != 0;
    m_preview_data = std::vector<std::vector<int>>(3, std::vector<int>(m_width*m_height,0)); // 2D vector of brightness values - first index = color, second index = pixel
    m_additional_layers_preview = has_additional_layers ? std::vector<std::vector<short int>>(3, std::vector<short int>(m_width*m_height, -1)) : std::vector<std::vector<short int>>();

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

                if (has_additional_layers && i_color == 0) {
                    const bool valid_pixel = m_additional_layers_data[0][index] >= 0;
                    if (valid_pixel) {
                        m_additional_layers_preview[0][index_new] = m_additional_layers_data[0][index];
                        m_additional_layers_preview[1][index_new] = m_additional_layers_data[1][index];
                        m_additional_layers_preview[2][index_new] = m_additional_layers_data[2][index];
                    }

                }
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

    // Check if the mouse is over the wxGenericStaticBitmap
    if (wxRect(m_preview_bitmap->GetSize()).Contains(screen_pos)) {
        // Get the amount of rotation
        int rotation = event.GetWheelRotation();

        // Calculate the relative position of the mouse within the wxGenericStaticBitmap
        wxSize bitmapSize = m_preview_bitmap->GetSize();
        float relative_x = static_cast<float>(screen_pos.x) / bitmapSize.GetWidth();
        float relative_y = static_cast<float>(screen_pos.y) / bitmapSize.GetHeight();

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

void ImagePreview::bind_shift_events()    {
    static std::pair<float, float> position_mouse_click = {-1, -1};

    m_preview_bitmap->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }

        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();

        // Check if the mouse is over the wxGenericStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(screen_pos)) {
            // Calculate the relative position of the mouse within the wxGenericStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(screen_pos.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(screen_pos.y) / bitmapSize.GetHeight();

            position_mouse_click = {relative_x, relative_y};
        }
    });

    m_preview_bitmap->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }

        if (position_mouse_click.first < 0 || position_mouse_click.second < 0) {
            return;
        }

        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();

        // Check if the mouse is over the wxGenericStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(screen_pos)) {
            // Calculate the relative position of the mouse within the wxGenericStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(screen_pos.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(screen_pos.y) / bitmapSize.GetHeight();

            const float dx = relative_x - position_mouse_click.first;
            const float dy = relative_y - position_mouse_click.second;

            update_preview_data(0.5-dx, 0.5-dy);
        }

        update_preview_bitmap();
    });
};


void ImagePreview::update_additional_layers_data()  {
    m_additional_layers_data = std::vector<std::vector<short int>>(3, std::vector<short int>(m_image_resize_tool.get_width_original()*m_image_resize_tool.get_height_original(), -1));
    for (const auto &layer_name_and_functor : m_additional_layers_functors) {
        layer_name_and_functor.second(&m_additional_layers_data, m_image_resize_tool.get_width_original(), m_image_resize_tool.get_height_original());
    }
    update_preview_data();
    update_preview_bitmap();
};