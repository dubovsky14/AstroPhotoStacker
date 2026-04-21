#pragma once

#include "../headers/ImagePreview.h"

#include "../../headers/LightPollutionGradientFunctions.h"
#include "../../headers/LightPollutionRemovalTool.h"

#include <vector>
#include <utility>
#include <array>

class ImagePreviewGridSelector : public ImagePreview {
    public:
        /**
         * @brief Construct a new Image Preview object
         *
         * @param width width of the preview
         * @param height height of the preview
         * @param max_value maximum value of the pixel
         * @param use_color_interpolation whether to use color interpolation when resizing the image
        */
        ImagePreviewGridSelector(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation);

        virtual void update_preview_bitmap() override;

        void set_grid_windows(const std::vector<AstroPhotoStacker::SampleWindow> &grid_windows);

        std::vector<AstroPhotoStacker::SampleWindow> get_selected_grid_windows() const;

    protected:


        void update_crop_preview(float mouse_position_relative_x = 0.5, float mouse_position_relative_y = 0.5);

        void plot_sample_windows(wxImage *xw_image) const;

        void plot_full_rectangle(wxImage *xw_image, int x1, int y1, int x2, int y2, const std::array<int, 3> &color) const;

        void bind_window_selection_events();

        int  get_grid_window_index_at_position(float relative_x, float relative_y) const;

        std::vector<std::pair<AstroPhotoStacker::SampleWindow, bool>> m_grid_windows_coordinates_and_validity; // coordinates and validity of the grid windows (validity is used to determine whether the window is selected or not)
};