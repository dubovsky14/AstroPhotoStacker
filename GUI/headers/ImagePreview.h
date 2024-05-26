#pragma once

#include "../headers/CombinedColorStrecherTool.h"

#include <vector>
#include <string>

#include <wx/wx.h>

/**
 * @brief Class responsible for image preview. It can read preview from a file or from a stacked image and apply exposure correction, color stretching and zooming.
*/
class ImagePreview {
    public:
        ImagePreview() = delete;

        /**
         * @brief Construct a new Image Preview object
         *
         * @param width width of the preview
         * @param height height of the preview
         * @param max_value maximum value of the pixel
         * @param use_color_interpolation whether to use color interpolation when resizing the image
        */
        ImagePreview(int width, int height, int max_value, bool use_color_interpolation);

        /**
         * @brief Is an image loaded?
         *
         * @return true if the image is loaded
        */
        bool image_loaded() const   { return m_width_original > 0 && m_height_original > 0;};

        /**
         * @brief Get the width of the preview
         *
         * @return int width of the preview in pixels
        */
        int get_width() const       { return m_width;};

        /**
         * @brief Get the height of the preview
         *
         * @return int height of the preview in pixels
        */
        int get_height() const      { return m_height;};

        /**
         * @brief Get the maximum value of the pixel
         *
         * @return int maximum value of the pixel
        */
        int get_max_value() const   { return m_max_value;};

        /**
         * @brief Load an image data from a file in a disk
        */
        void read_preview_from_file(const std::string &path);

        /**
         * @brief Load preview from a stacked image
         *
         * @param stacked_image original stacked image
         * @param width_original width of the original image
         * @param height_original height of the original image
        */
        void read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original);

        /**
         * @brief Set exposure correction
         *
         * @param exposure_correction exposure correction value
        */
        void set_exposure_correction(float exposure_correction) { m_exposure_correction = exposure_correction;};

        /**
         * @brief Get exposure correction value
         *
         * @return float exposure correction value
        */
        float get_exposure_correction() const { return m_exposure_correction;};

        /**
         * @brief Update preview bitmap - an object in GUI
         *
         * @param static_bitmap pointer to static bitmap to update
        */
        void update_preview_bitmap(wxStaticBitmap *static_bitmap) const;

        /**
         * @brief Zoom in the preview at a given position
         *
         * @param mouse_position_relative_x relative x position of the mouse
         * @param mouse_position_relative_y relative y position of the mouse
        */
        void zoom_in(float mouse_position_relative_x, float mouse_position_relative_y);

        /**
         * @brief Zoom out the preview at a given position
         *
         * @param mouse_position_relative_x relative x position of the mouse
         * @param mouse_position_relative_y relative y position of the mouse
        */
        void zoom_out(float mouse_position_relative_x, float mouse_position_relative_y);

        /**
         * @brief Set color stretcher for the image
         *
         * @param color_stretcher pointer to the stretcher
        */
        void set_stretcher(const CombinedColorStrecherTool *color_stretcher);

        /**
         * @brief Get preview data as vector<vector<int>> (3 color channels, each with width*height pixels)
         *
         * @return const std::vector<std::vector<int>>& preview data
        */
        const std::vector<std::vector<int>>& get_preview_data() const { return m_preview_data;};

        /**
         * @brief Get original image data as vector<vector<short unsigned int>> (3 color channels, each with width*height pixels)
         *
         * @return  const std::vector<std::vector<short unsigned int>>& original image data
        */
        const std::vector<std::vector<short unsigned int>>& get_original_image() const { return m_original_image;};

    private:
        int m_width_original    = 0;
        int m_height_original   = 0;
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