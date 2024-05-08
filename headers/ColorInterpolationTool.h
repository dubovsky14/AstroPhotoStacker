#pragma once

#include <vector>


namespace AstroPhotoStacker {
    class ColorInterpolationTool {
        public:
            ColorInterpolationTool() = delete;

            ColorInterpolationTool(const short unsigned int *data_original, int width, int height, const std::vector<char> &colors_original, const std::vector<char> &color_conversion_table);

            std::vector<std::vector<short unsigned int>> get_interpolated_rgb_image() const;

        private:
            int m_width;
            int m_height;

            const short unsigned int *m_data_original = nullptr;
            const std::vector<char> *m_colors_original  = nullptr;
            const std::vector<char> *m_color_conversion_table   = nullptr;


            void get_closest_pixel_of_given_color(  int pos_x,
                                                    int pos_y,
                                                    int color,
                                                    int step_x,
                                                    int step_y,
                                                    short unsigned int *value,
                                                    int *closest_distance,
                                                    int n_steps_max = 2) const;
    };
}