#pragma once

#include "../headers/CombinedColorStrecherTool.h"
#include "../headers/ImageResizeTool.h"

#include "../../headers/InputFrame.h"
#include "../../headers/PixelType.h"
#include "../../headers/LensCorrectionsTool.h"

#include <vector>
#include <string>
#include <functional>
#include <map>

#include <wx/wx.h>
#include <wx/generic/statbmpg.h>

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
        ImagePreview(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation);


        /**
         * @brief Get the maximum zoom factor
         *
         * @return double maximum zoom factor
         */
        void set_max_zoom_factor(double max_zoom_factor) { m_max_zoom_factor = max_zoom_factor;};

        /**
         * @brief Is an image loaded?
         *
         * @return true if the image is loaded
        */
        bool image_loaded() const   { return m_image_resize_tool.get_width_original() > 0 && m_image_resize_tool.get_height_original() > 0;};

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
        void read_preview_from_file(const std::string &path, const AstroPhotoStacker::LensCorrectionsTool *lens_corrections_tool = nullptr);

        /**
         * @brief Load an image data from a file in a disk
        */
        void read_preview_from_frame(const AstroPhotoStacker::InputFrame &input_frame, const AstroPhotoStacker::LensCorrectionsTool *lens_corrections_tool = nullptr);

        /**
         * @brief Load preview from a stacked image
         *
         * @param stacked_image original stacked image
         * @param width_original width of the original image
         * @param height_original height of the original image
        */
        void read_preview_from_stacked_image(const std::vector<std::vector<double>> &stacked_image, int width_original, int height_original);

        /**
         * @brief Update original image
         *
         * @param original_image original image data
         * @param width width of the image
         * @param height height of the image
         */
        void update_original_image(const std::vector<std::vector<PixelType>> &original_image, int width, int height);

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
        virtual void update_preview_bitmap();

        /**
         * @brief Zoom in the preview at a given position
         *
         * @param mouse_position_relative_x relative x position of the mouse
         * @param mouse_position_relative_y relative y position of the mouse
         * @param zoom_step zoom step
        */
        void zoom(float mouse_position_relative_x, float mouse_position_relative_y, float zoom_step);

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
         * @brief Get original image data as vector<vector<PixelType>> (3 color channels, each with width*height pixels)
         *
         * @param width pointer to the width of the image
         * @param height pointer to the height of the image
         *
         * @return  const std::vector<std::vector<PixelType>>& original image data
        */
        const std::vector<std::vector<PixelType>>& get_original_image(int *width = nullptr, int *height = nullptr) const;

        /**
         * @brief Image preview bitmap (for sizer)
        */
        wxGenericStaticBitmap *get_image_preview_bitmap()   const   {return m_preview_bitmap;};

        /**
         * @brief Add additional layer to the image preview - for example to show alignment boxes or crop borders
         *
         * @param layer_name name of the layer
         * @param functor function to apply the layer
        */
        void add_layer(const std::string &layer_name, const std::function<void(std::vector<std::vector<PixelType>> *, int, int)> &functor);

        /**
         * @brief Remove additional layer
        */
        void remove_layer(const std::string &layer_name);

    protected:
        wxWindow *m_parent = nullptr;
        wxGenericStaticBitmap                  *m_preview_bitmap       = nullptr;
        ImageResizeTool m_image_resize_tool;


        std::vector<std::vector<PixelType>>                     m_original_image;               // 3 color channels, each with width*height pixels
        std::vector<std::vector<PixelType>>                     m_additional_layers_data;       // 3 color channels, each with width*height pixels
        std::vector<std::vector<PixelType>>                     m_additional_layers_preview;    // 3 color channels, each with width*height pixels = -1 means empty pixel in the additional layer

        // vector of functors to apply additional layers to the image, such as crop borders, alignment points, etc.
        std::map<std::string, std::function<void(std::vector<std::vector<PixelType>> *, int, int)>>   m_additional_layers_functors;

        int m_width = 0;
        int m_height =0;
        std::vector<std::vector<int>> m_preview_data; // 3 color channels, each with width*height pixels
        float m_exposure_correction;
        int m_max_value;
        std::vector<int> m_max_values_original;
        const CombinedColorStrecherTool *m_color_stretcher = nullptr;

        double m_zoom_factor = 1.0;
        double m_max_zoom_factor = 8.0;
        double m_min_zoom_factor = 1;
        double m_zoom_factor_one_to_one = 100; // zoom factor for which one pixel in preview corresponds to one pixel in original image

        void initialize_bitmap();

        void set_default_resized_area();
        void update_max_values_original();

        void update_preview_data(float mouse_position_relative_x = 0.5, float mouse_position_relative_y = 0.5);

        void on_mouse_wheel(wxMouseEvent& event);

        wxImage get_updated_wximage() const;

        void bind_shift_events();

        void update_additional_layers_data();

        int get_interpolated_original_image_data(const std::vector<PixelType> &original_image_channel_data, float x, float y) const;

        // Hook for derived classes to react on original image update
        virtual void on_update_original_image() {};
};