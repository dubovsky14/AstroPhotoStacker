#include <stdexcept>

#include "../headers/ImageResizeTool.h"

using namespace std;

void ImageResizeTool::set_original_size(int width, int height) {
    m_width_original = width;
    m_height_original = height;
};

void ImageResizeTool::set_preview_size(int width, int height) {
    m_width = width;
    m_height = height;
};

void ImageResizeTool::set_zoom_factor(float zoom_factor) {
    m_zoom_factor = zoom_factor;
};

void ImageResizeTool::set_relative_mouse_position(float mouse_position_relative_x, float mouse_position_relative_y) {
    m_mouse_position_relative_x = mouse_position_relative_x;
    m_mouse_position_relative_y = mouse_position_relative_y;
};

void ImageResizeTool::update()    {
    if (m_width_original == -1 || m_height_original == -1) {
        throw std::runtime_error("Original size not set");
    }

    if (m_width == -1 || m_height == -1) {
        throw std::runtime_error("Preview size not set");
    }

    if (m_zoom_factor == -1) {
        throw std::runtime_error("Zoom factor not set");
    }

    if (m_mouse_position_relative_x == -1 || m_mouse_position_relative_y == -1) {
        throw std::runtime_error("Mouse position not set");
    }

    m_step_x =  m_width_original  / (m_zoom_factor*float(m_width) );
    m_step_y =  m_height_original / (m_zoom_factor*float(m_height));

    int x_center = m_i_x_resized_min + (m_i_x_resized_max - m_i_x_resized_min)*m_mouse_position_relative_x;
    if (x_center < m_width_original/(m_zoom_factor*2)) {
        x_center = m_width_original/(m_zoom_factor*2);
    }
    if (x_center > m_width_original - m_width_original/(m_zoom_factor*2)) {
        x_center = m_width_original - m_width_original/(m_zoom_factor*2);
    }

    int y_center = m_i_y_resized_min + (m_i_y_resized_max - m_i_y_resized_min)*m_mouse_position_relative_y;
    if (y_center < m_height_original/(m_zoom_factor*2)) {
        y_center = m_height_original/(m_zoom_factor*2);
    }
    if (y_center > m_height_original - m_height_original/(m_zoom_factor*2)) {
        y_center = m_height_original - m_height_original/(m_zoom_factor*2);
    }

    m_i_x_resized_min = max<int>(0, 0.5 + x_center - m_width_original/(m_zoom_factor*2));
    m_i_x_resized_max = min<int>(m_width_original, 0.5 + x_center + m_width_original/(m_zoom_factor*2));

    m_i_y_resized_min = max<int>(0, 0.5 + y_center - m_height_original/(m_zoom_factor*2));
    m_i_y_resized_max = min<int>(m_height_original, 0.5 + y_center + m_height_original/(m_zoom_factor*2));
};

int ImageResizeTool::get_preview_coordinate_x(int x_original) const {
    return (x_original-m_i_x_resized_min) / m_step_x;
};

int ImageResizeTool::get_preview_coordinate_y(int y_original) const {
    return (y_original-m_i_y_resized_min) / m_step_y;
};

float ImageResizeTool::get_original_coordinate_x(float x_relative) const   {
    float x_center = m_i_x_resized_min + (m_i_x_resized_max - m_i_x_resized_min)*x_relative;
    if (x_center < m_i_x_resized_min) {
        x_center = m_i_x_resized_min;
    }
    if (x_center > m_i_x_resized_max) {
        x_center = m_i_x_resized_max;
    }
    return x_center;
};

float ImageResizeTool::get_original_coordinate_y(float y_relative) const   {
    float y_center = m_i_y_resized_min + (m_i_y_resized_max - m_i_y_resized_min)*y_relative;
    if (y_center < m_i_y_resized_min) {
        y_center = m_i_y_resized_min;
    }
    if (y_center > m_i_y_resized_max) {
        y_center = m_i_y_resized_max;
    }
    return y_center;
};

std::tuple<int, int> ImageResizeTool::get_preview_coordinates(int x, int y) const    {
    return make_tuple(get_preview_coordinate_x(x), get_preview_coordinate_y(y));
};

void ImageResizeTool::get_crop_borders_in_original_coordinates(
    int *x_resized_min,
    int *x_resized_max,
    int *y_resized_min,
    int *y_resized_max) const {

    *x_resized_min = m_i_x_resized_min;
    *x_resized_max = m_i_x_resized_max;
    *y_resized_min = m_i_y_resized_min;
    *y_resized_max = m_i_y_resized_max;
};

void ImageResizeTool::set_default_resized_area()   {
    if (m_i_x_resized_max < 0)  {
        m_i_x_resized_min = 0;
        m_i_x_resized_max = m_width_original;
        m_i_y_resized_min = 0;
        m_i_y_resized_max = m_height_original;
    }
};