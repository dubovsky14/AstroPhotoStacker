#include "../headers/StackerKappaSigmaClipping.h"

#include <cmath>


using namespace std;
using namespace AstroPhotoStacker;

StackerKappaSigmaClipping::StackerKappaSigmaClipping(int number_of_colors, int width, int height) :
    StackerMedian(number_of_colors, width, height)   {
};

void StackerKappaSigmaClipping::set_kappa(float kappa) {
    m_kappa = kappa;
}

void StackerKappaSigmaClipping::set_number_of_iterations(int n_iterations) {
    m_n_iterations = n_iterations;
}

void StackerKappaSigmaClipping::process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)    {
    const int n_files = m_files_to_stack.size();
    for (int i_width = 0; i_width < m_width; i_width++) {
        const unsigned long long int pixel_index = m_width*y_index_final_array + i_width;
        const unsigned long long int pixel_index_stacking_array = m_width*y_index_values_to_stack_array*n_files + i_width*n_files;

        short *slice_begin = &m_values_to_stack[i_color][pixel_index_stacking_array];


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

        double mean(0);
        int n = 0;
        for (int i_file = 0; i_file < n_files; i_file++) {
            if (slice_begin[i_file] >= 0) {
                mean += slice_begin[i_file];
                n++;
            }
        }
        mean /= n;
        m_stacked_image[i_color][pixel_index] = mean;
    }
};