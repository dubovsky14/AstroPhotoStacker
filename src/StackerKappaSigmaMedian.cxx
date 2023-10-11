#include "../headers/StackerKappaSigmaMedian.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaMedian::StackerKappaSigmaMedian(int number_of_colors, int width, int height) :
    StackerKappaSigmaBase(number_of_colors, width, height)   {
};

void StackerKappaSigmaMedian::process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)    {
    const int n_files = m_files_to_stack.size();
    for (int i_width = 0; i_width < m_width; i_width++) {
        const unsigned long long int pixel_index = m_width*y_index_final_array + i_width;
        const unsigned long long int pixel_index_stacking_array = m_width*y_index_values_to_stack_array*n_files + i_width*n_files;

        short *slice_begin = &m_values_to_stack[i_color][pixel_index_stacking_array];
        short *slice_end   = &m_values_to_stack[i_color][pixel_index_stacking_array + n_files];


        for (int i_iter = 0; i_iter < m_n_iterations; i_iter++)    {
            double mean(0), mean2(0);
            int n = 0;
            for (int i_file = 0; i_file < n_files; i_file++) {
                if (slice_begin[i_file] >= 0) {
                    mean += slice_begin[i_file];
                    mean2 += slice_begin[i_file]*slice_begin[i_file];
                    n++;
                }
            }
            mean /= n;
            mean2 /= n;
            const double sigma = sqrt(mean2 - mean*mean);
            const double kappa_sigma = m_kappa*sigma;
            for (int i_file = 0; i_file < n_files; i_file++) {
                if (slice_begin[i_file] >= 0) {
                    if (abs(slice_begin[i_file] - mean) > kappa_sigma) {
                        slice_begin[i_file] = -1;
                    }
                }
            }
        }

        // calculating median
        int number_of_stacked_pixels = 0;
        for (int i_file = 0; i_file < n_files; i_file++) {
            if (slice_begin[i_file] >= 0) {
                number_of_stacked_pixels++;
            }
        }
        const unsigned long long int start_of_non_negative_values = pixel_index_stacking_array + n_files - number_of_stacked_pixels;

        sort(slice_begin, slice_end);

        if (number_of_stacked_pixels == 0) {
            m_stacked_image[i_color][pixel_index] = 0;
        }
        else if (number_of_stacked_pixels % 2 == 0) {
            m_stacked_image[i_color][pixel_index] = (m_values_to_stack[i_color][start_of_non_negative_values + number_of_stacked_pixels/2] + m_values_to_stack[i_color][start_of_non_negative_values + number_of_stacked_pixels/2 - 1])/2;
        }
        else {
            m_stacked_image[i_color][pixel_index] = m_values_to_stack[i_color][start_of_non_negative_values + number_of_stacked_pixels/2];
        }
    }
};