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

int StackerMaximum::get_height_range_limit() const {
    int height_range = m_height;
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_needed_for_stacked_image = 3*sizeof(double)*m_width*m_height;
        const unsigned long long int memory_needed_for_calibrated_photos = m_n_cpu*3*sizeof(unsigned short)*m_width*m_height;
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - memory_needed_for_stacked_image - memory_needed_for_calibrated_photos;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_n_cpu*12ULL*m_width;
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};