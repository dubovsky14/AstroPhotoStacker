#pragma once

#include <tuple>

class ImageResizeTool   {
    public:
        ImageResizeTool() = default;

        ImageResizeTool(const ImageResizeTool&) = default;

        void set_original_size(int width, int height);

        void set_preview_size(int width, int height);

        void set_zoom_factor(float zoom_factor);

        void set_relative_mouse_position(float mouse_position_relative_x, float mouse_position_relative_y);

        void update();

        int get_preview_coordinate_x(int x_original) const;

        int get_preview_coordinate_y(int y_original) const;

        float get_original_coordinate_x(float x_relative) const;

        float get_original_coordinate_y(float y_relative) const;

        std::tuple<int, int> get_preview_coordinates(int x, int y) const;

        void get_crop_borders_in_original_coordinates(int *x_resized_min, int *x_resized_max, int *y_resized_min, int *y_resized_max) const;

        int get_width_original() const { return m_width_original; };

        int get_height_original() const { return m_height_original; };

        float get_last_mouse_position_relative_x() const { return m_mouse_position_relative_x; };

        float get_last_mouse_position_relative_y() const { return m_mouse_position_relative_y; };

        void set_default_resized_area();

    private:
        int m_width_original = -1;
        int m_height_original = -1;

        int m_width = -1;
        int m_height = -1;

        float m_zoom_factor = -1;

        float m_mouse_position_relative_x = -1;
        float m_mouse_position_relative_y = -1;

        float m_step_x = -1;
        float m_step_y = -1;

        int m_i_x_resized_min = -1;
        int m_i_x_resized_max = -1;

        int m_i_y_resized_min = -1;
        int m_i_y_resized_max = -1;
};