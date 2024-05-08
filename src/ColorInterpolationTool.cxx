#include "../headers/ColorInterpolationTool.h"

using namespace std;
using namespace AstroPhotoStacker;

ColorInterpolationTool::ColorInterpolationTool(const short unsigned int *data_original, int width, int height, const std::vector<char> &colors_original, const std::vector<char> &color_conversion_table)    {
    m_width = width;
    m_height = height;
    m_data_original = data_original;
    m_colors_original = &colors_original;
    m_color_conversion_table = &color_conversion_table;
}

void ColorInterpolationTool::get_closest_pixel_of_given_color(  int pos_x,
                                                                int pos_y,
                                                                int color,
                                                                int step_x,
                                                                int step_y,
                                                                short unsigned int *value,
                                                                int *closest_distance,
                                                                int n_steps_max) const {

    *closest_distance = -1;
    *value = 0;

    for (int i_step = 1; i_step <= n_steps_max; i_step++)  {
        const int new_pos_x = pos_x + i_step*step_x;
        const int new_pos_y = pos_y + i_step*step_y;
        if (new_pos_x < 0 || new_pos_x >= m_width || new_pos_y < 0 || new_pos_y >= m_height) {
            return;
        }
        const int index = new_pos_y*m_width + new_pos_x;
        if ((*m_color_conversion_table)[(*m_colors_original)[index]] == color) {
            *closest_distance = i_step;
            *value = m_data_original[index];
            return;
        }
    }
};