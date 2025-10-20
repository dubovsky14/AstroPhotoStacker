#pragma once

#include "../headers/ImagePreview.h"

class ImagePreviewCropTool : public ImagePreview {
    public:
        /**
         * @brief Construct a new Image Preview object
         *
         * @param width width of the preview
         * @param height height of the preview
         * @param max_value maximum value of the pixel
         * @param use_color_interpolation whether to use color interpolation when resizing the image
        */
        ImagePreviewCropTool(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation);

        virtual void update_preview_bitmap() override;

        void get_crop_coordinates(int *top_left_x, int *top_left_y, int *width, int *height) const;

        void drop_crop();

    protected:


        void update_crop_preview(float mouse_position_relative_x = 0.5, float mouse_position_relative_y = 0.5);

        void plot_crop_rectangle(wxImage *xw_image) const;

        void plot_full_rectangle(wxImage *xw_image, int x1, int y1, int x2, int y2) const;

        void bind_crop_events();

        int m_crop_top_left_x = 0;
        int m_crop_top_left_y = 0;

        int m_crop_width  = -1;
        int m_crop_height = -1;
};