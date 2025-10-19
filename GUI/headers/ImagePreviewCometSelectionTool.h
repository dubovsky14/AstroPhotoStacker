#pragma once

#include "../headers/ImagePreview.h"


class ImagePreviewCometSelectionTool : public ImagePreview {
    public:
        /**
         * @brief Construct a new Image Preview object
         *
         * @param width width of the preview
         * @param height height of the preview
         * @param max_value maximum value of the pixel
         * @param use_color_interpolation whether to use color interpolation when resizing the image
        */
        ImagePreviewCometSelectionTool(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation);

        virtual void update_preview_bitmap() override;

        std::pair<float,float> get_comet_position() const { return m_comet_position; };

        void set_comet_position(float x, float y);

    protected:

        std::vector<std::pair<float,float>> calculate_star_positions();

        std::pair<float,float> m_comet_position = std::make_pair(-1.0f, -1.0f);

        std::vector<std::pair<float,float>> m_star_positions; // calculated star positions

        virtual void on_update_original_image() override;

        void plot_stars();

        void add_circle_to_additional_layers(float center_x, float center_y, float radius_outer, float radius_inner, std::array<int,3> color);

        void bind_comet_selection_events();

        int get_closest_star_index(float x, float y) const {
            int closest_index = -1;
            float min_distance_squared = std::numeric_limits<float>::max();
            for (size_t i = 0; i < m_star_positions.size(); ++i) {
                const float dx = m_star_positions[i].first - x;
                const float dy = m_star_positions[i].second - y;
                const float distance_squared = dx * dx + dy * dy;
                if (distance_squared < min_distance_squared) {
                    min_distance_squared = distance_squared;
                    closest_index = static_cast<int>(i);
                }
            }
            return closest_index;
        };
};