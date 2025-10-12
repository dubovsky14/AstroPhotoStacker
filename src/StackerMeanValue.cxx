#include "../headers/StackerMeanValue.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerMeanValue::StackerMeanValue(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerSimpleBase(number_of_colors, width, height, interpolate_colors)  {};

void StackerMeanValue::allocate_arrays_for_stacking(int dy) {
    m_values_to_stack_individual_threads = vector<vector<vector<int>>>(m_n_cpu, vector<vector<int>>(m_number_of_colors, vector<int>(m_width*dy, 0)));
    m_counts_to_stack_individual_threads = vector<vector<vector<short unsigned int>>>(m_n_cpu, vector<vector<short unsigned int>>(m_number_of_colors, vector<short unsigned int>(m_width*dy, 0)));
};

void StackerMeanValue::reset_values_in_arrays_for_stacking()  {
    for (unsigned int i_thread = 0; i_thread < m_n_cpu; i_thread++) {
        vector<vector<int>>                 &stacked_image = m_values_to_stack_individual_threads[i_thread];
        vector<vector<short unsigned int>>  &number_of_stacked_pixels = m_counts_to_stack_individual_threads[i_thread];

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (unsigned int i_pixel = 0; i_pixel < stacked_image[i_color].size(); i_pixel++) {
                stacked_image[i_color][i_pixel] = 0;
                number_of_stacked_pixels[i_color][i_pixel] = 0;
            }
        }
    }
};

void StackerMeanValue::deallocate_arrays_for_stacking() {
    m_values_to_stack_individual_threads.clear();
    m_counts_to_stack_individual_threads.clear();
};

void StackerMeanValue::process_pixel(int i_color, int i_pixel, int value, int i_thread) {
    m_values_to_stack_individual_threads[i_thread][i_color][i_pixel] += value;
    m_counts_to_stack_individual_threads[i_thread][i_color][i_pixel] += 1;
};

void StackerMeanValue::calculate_final_image(int y_min, int y_max)    {
    const int pixel_shift = y_min*m_width;
    const int dy = y_max - y_min;
    std::vector<std::vector<unsigned short>> number_of_stacked_pixels_total(m_number_of_colors, vector<unsigned short>(m_width*dy, 0));

    // sum partial results
    for (unsigned int i_thread = 0; i_thread < m_n_cpu; i_thread++) {
        vector<vector<int>>                 &stacked_image = m_values_to_stack_individual_threads[i_thread];
        vector<vector<short unsigned int>>  &number_of_stacked_pixels = m_counts_to_stack_individual_threads[i_thread];

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int i_pixel = 0; i_pixel < m_width*dy; i_pixel++) {
                m_stacked_image[i_color][i_pixel+pixel_shift] += stacked_image[i_color][i_pixel];
                number_of_stacked_pixels_total[i_color][i_pixel] += number_of_stacked_pixels[i_color][i_pixel];
            }
        }
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < m_width*dy; i_pixel++) {
            if (number_of_stacked_pixels_total[i_color][i_pixel] > 0) {
                m_stacked_image[i_color][i_pixel+pixel_shift] /= number_of_stacked_pixels_total[i_color][i_pixel];
            }
            else {
                m_stacked_image[i_color][i_pixel+pixel_shift] = c_empty_pixel_value;
            }
        }
    }
}

int StackerMeanValue::get_height_range_limit() const {
    int height_range = m_height;
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_needed_for_stacked_image = 3*sizeof(double)*m_width*m_height;
        const unsigned long long int memory_needed_for_calibrated_photos = m_n_cpu*3*sizeof(PixelType)*m_width*m_height;
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - memory_needed_for_stacked_image - memory_needed_for_calibrated_photos;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_n_cpu*18ULL*m_width;
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};