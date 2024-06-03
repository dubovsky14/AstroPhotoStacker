#include "../headers/StackerMaximum.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerMaximum::StackerMaximum(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerSimpleBase(number_of_colors, width, height, interpolate_colors)  {};

void StackerMaximum::allocate_arrays_for_stacking(int dy) {
    m_max_values_individual_threads = vector<vector<vector<int>>>(m_n_cpu, vector<vector<int>>(m_number_of_colors, vector<int>(m_width*dy, c_empty_pixel_value)));
};

void StackerMaximum::reset_values_in_arrays_for_stacking() {
    for (vector<vector<int>> &stacked_image : m_max_values_individual_threads) {
        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (unsigned int i_pixel = 0; i_pixel < stacked_image[i_color].size(); i_pixel++) {
                stacked_image[i_color][i_pixel] = c_empty_pixel_value;
            }
        }
    }
};

void StackerMaximum::deallocate_arrays_for_stacking() {
    m_max_values_individual_threads.clear();
};

void StackerMaximum::process_pixel(int i_color, int i_pixel, int value, int i_thread) {
    m_max_values_individual_threads[i_thread][i_color][i_pixel] = std::max<int>(value, m_max_values_individual_threads[i_thread][i_color][i_pixel]);
};

void StackerMaximum::calculate_final_image(int y_min, int y_max)    {
    const int dy = y_max - y_min;
    // sum partial results
    const int pixel_shift = y_min*m_width;
    for (unsigned int i_thread = 0; i_thread < m_n_cpu; i_thread++) {
        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int i_pixel = 0; i_pixel < m_width*dy; i_pixel++) {
                m_stacked_image[i_color][i_pixel+pixel_shift] = std::max<int>(m_max_values_individual_threads[i_thread][i_color][i_pixel], m_stacked_image[i_color][i_pixel+pixel_shift]);
            }
        }
    }
}