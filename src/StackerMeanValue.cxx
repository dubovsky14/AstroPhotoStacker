#include "../headers/StackerMeanValue.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/CustomScopeMutex.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

StackerMeanValue::StackerMeanValue(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerBase(number_of_colors, width, height, interpolate_colors),
    m_number_of_stacked_pixels(number_of_colors, vector<unsigned short>(width*height, 0))   {
};

void StackerMeanValue::allocate_arrays_for_stacking() {
    m_values_to_stack_individual_threads = vector<vector<vector<int>>>(m_n_cpu, vector<vector<int>>(m_number_of_colors, vector<int>(m_width*m_height, 0)));
    m_counts_to_stack_individual_threads = vector<vector<vector<short unsigned int>>>(m_n_cpu, vector<vector<short unsigned int>>(m_number_of_colors, vector<short unsigned int>(m_width*m_height, 0)));
};

void StackerMeanValue::calculate_stacked_photo()  {
    m_mutexes = vector<mutex>(m_n_cpu);

    allocate_arrays_for_stacking();

    int y_min = 0;
    int y_max = get_height_range_limit();
    if (m_n_cpu == 1)   {
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            add_photo_to_stack(i_file, y_min, y_max);
        }
    }
    else    {
        thread_pool pool(m_n_cpu);
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            pool.submit([this, i_file, y_min, y_max]() {
                add_photo_to_stack(i_file, y_min, y_max);
            });
        }
        pool.wait_for_tasks();
    }

    calculate_final_image();

    fix_empty_pixels();

    deallocate_arrays_for_stacking();
};

void StackerMeanValue::deallocate_arrays_for_stacking() {
    m_values_to_stack_individual_threads.clear();
    m_counts_to_stack_individual_threads.clear();
};

void StackerMeanValue::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};

void StackerMeanValue::add_photo_to_stack(unsigned int i_file, int y_min, int y_max)  {
    cout << "Adding " + m_files_to_stack[i_file] + " to stack\n";
    CalibratedPhotoHandler calibrated_photo = get_calibrated_photo(i_file, y_min, y_max);

    unsigned int i_thread = 0;
    while (true) {
        i_thread = (i_thread+1) % m_mutexes.size();
        CustomScopeMutex scope_mutex(&m_mutexes[i_thread]);
        if (!scope_mutex.is_locked()) {
            continue;
        }

        vector<vector<int>>                 &stacked_image = m_values_to_stack_individual_threads[i_thread];
        vector<vector<short unsigned int>>  &number_of_stacked_pixels = m_counts_to_stack_individual_threads[i_thread];

        unsigned int value;
        char color;
        for (int y = 0; y < m_height; y++)  {
            for (int x = 0; x < m_width; x++)   {
                calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
                if (color >= 0) {
                    const unsigned int index = y*m_width + x;
                    stacked_image[color][index]   += value;
                    number_of_stacked_pixels[color][index] += 1;
                }
            }
        }
        break;
    }

    m_n_tasks_processed++;
};

int StackerMeanValue::get_height_range_limit() const  {
    return m_width;
};

int StackerMeanValue::get_tasks_total() const  {
    return m_files_to_stack.size();
};

void StackerMeanValue::calculate_final_image()    {
    // sum partial results
    for (unsigned int i_thread = 0; i_thread < m_n_cpu; i_thread++) {
        vector<vector<int>>                 &stacked_image = m_values_to_stack_individual_threads[i_thread];
        vector<vector<short unsigned int>>  &number_of_stacked_pixels = m_counts_to_stack_individual_threads[i_thread];

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
                m_stacked_image[i_color][i_pixel] += stacked_image[i_color][i_pixel];
                m_number_of_stacked_pixels[i_color][i_pixel] += number_of_stacked_pixels[i_color][i_pixel];
            }
        }
        stacked_image.clear();
        number_of_stacked_pixels.clear();
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            if (m_number_of_stacked_pixels[i_color][i_pixel] > 0) {
                m_stacked_image[i_color][i_pixel] /= m_number_of_stacked_pixels[i_color][i_pixel];
            }
            else {
                m_stacked_image[i_color][i_pixel] = c_empty_pixel_value;
            }
        }
    }
}