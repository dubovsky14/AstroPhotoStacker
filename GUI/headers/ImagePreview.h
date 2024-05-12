#pragma once

#include "../headers/ColorStretching.h"

#include <vector>
#include <string>

#include <wx/wx.h>


class ImagePreview {
    public:
        ImagePreview() = delete;
        ImagePreview(int width, int height, int max_value, bool use_color_interpolation);

        void get_preview_from_stacked_picture(  const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original);

        int get_width() const       { return m_width;};
        int get_height() const      { return m_height;};
        int get_max_value() const   { return m_max_value;};

        void read_preview_from_file(const std::string &path);
        void read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original);

        void set_exposure_correction(float exposure_correction) { m_exposure_correction = exposure_correction;};
        float get_exposure_correction() const { return m_exposure_correction;};

        void update_preview_bitmap(wxStaticBitmap *static_bitmap) const;

        void zoom_in(float mouse_position_relative_x, float mouse_position_relative_y);
        void zoom_out(float mouse_position_relative_x, float mouse_position_relative_y);

        void set_stretcher(const CombinedColorStrecherTool *color_stretcher);

        const std::vector<std::vector<int>>& get_preview_data() const { return m_preview_data;};
        const std::vector<std::vector<short unsigned int>>& get_original_image() const { return m_original_image;};

    private:
        int m_width_original;
        int m_height_original;
        std::vector<std::vector<short unsigned int>> m_original_image; // 3 color channels, each with width*height pixels

        int m_width;
        int m_height;
        std::vector<std::vector<int>> m_preview_data; // 3 color channels, each with width*height pixels
        float m_exposure_correction;
        int m_max_value;
        std::vector<int> m_max_values_original;
        const CombinedColorStrecherTool *m_color_stretcher = nullptr;
        bool m_current_preview_is_raw_file = true;
        bool m_use_color_interpolation = true;

        double m_zoom_factor = 1.0;
        double m_max_zoom_factor = 8.0;
        double m_min_zoom_factor = 1;

        int m_i_x_resized_min = -1;
        int m_i_x_resized_max = -1;
        int m_i_y_resized_min = -1;
        int m_i_y_resized_max = -1;

        void set_default_resized_area();
        void update_max_values_original();

        void update_preview_data(float mouse_position_relative_x = 0.5, float mouse_position_relative_y = 0.5);
};