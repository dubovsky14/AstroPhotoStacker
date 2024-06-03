#include "../headers/StackerMeanValue.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerMeanValue::StackerMeanValue(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerSimpleBase(number_of_colors, width, height, interpolate_colors)  {};

void StackerMeanValue::allocate_arrays_for_stacking() {
    m_values_to_stack_individual_threads = vector<vector<vector<int>>>(m_n_cpu, vector<vector<int>>(m_number_of_colors, vector<int>(m_width*m_height, 0)));
    m_counts_to_stack_individual_threads = vector<vector<vector<short unsigned int>>>(m_n_cpu, vector<vector<short unsigned int>>(m_number_of_colors, vector<short unsigned int>(m_width*m_height, 0)));
};


void StackerMeanValue::deallocate_arrays_for_stacking() {
    m_values_to_stack_individual_threads.clear();
    m_counts_to_stack_individual_threads.clear();
};

void StackerMeanValue::process_pixel(int i_color, int i_pixel, int value, int i_thread) {
    m_values_to_stack_individual_threads[i_thread][i_color][i_pixel] += value;
    m_counts_to_stack_individual_threads[i_thread][i_color][i_pixel] += 1;
};

void StackerMeanValue::calculate_final_image()    {
    std::vector<std::vector<unsigned short>> number_of_stacked_pixels_total(m_number_of_colors, vector<unsigned short>(m_width*m_height, 0));

    // sum partial results
    for (unsigned int i_thread = 0; i_thread < m_n_cpu; i_thread++) {
        vector<vector<int>>                 &stacked_image = m_values_to_stack_individual_threads[i_thread];
        vector<vector<short unsigned int>>  &number_of_stacked_pixels = m_counts_to_stack_individual_threads[i_thread];

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
                m_stacked_image[i_color][i_pixel] += stacked_image[i_color][i_pixel];
                number_of_stacked_pixels_total[i_color][i_pixel] += number_of_stacked_pixels[i_color][i_pixel];
            }
        }
        stacked_image.clear();
        number_of_stacked_pixels.clear();
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            if (number_of_stacked_pixels_total[i_color][i_pixel] > 0) {
                m_stacked_image[i_color][i_pixel] /= number_of_stacked_pixels_total[i_color][i_pixel];
            }
            else {
                m_stacked_image[i_color][i_pixel] = c_empty_pixel_value;
            }
        }
    }
}