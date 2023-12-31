#include "../headers/StackerMeanValue.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/CustomScopeMutex.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

StackerMeanValue::StackerMeanValue(int number_of_colors, int width, int height) :
    StackerBase(number_of_colors, width, height),
    m_number_of_stacked_pixels(number_of_colors, vector<unsigned short>(width*height, 0))   {
};

void StackerMeanValue::calculate_stacked_photo()  {
    m_mutexes = vector<mutex>(m_n_cpu);
    m_values_to_stack_individual_threads = vector<vector<vector<int>>>(m_n_cpu, vector<vector<int>>(m_number_of_colors, vector<int>(m_width*m_height, 0)));
    m_counts_to_stack_individual_threads = vector<vector<vector<short unsigned int>>>(m_n_cpu, vector<vector<short unsigned int>>(m_number_of_colors, vector<short unsigned int>(m_width*m_height, 0)));

    if (m_n_cpu == 1)   {
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            add_photo_to_stack(i_file);
        }
    }
    else    {
        thread_pool pool(m_n_cpu);
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            pool.submit([this, i_file]() {
                add_photo_to_stack(i_file);
            });
        }
        pool.wait_for_tasks();
    }

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

    // fix green pixels
    if (m_number_of_colors == 3) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            m_stacked_image[1][i_pixel] /= 2;
        }
    }
    fix_empty_pixels();

    m_values_to_stack_individual_threads.clear();
    m_counts_to_stack_individual_threads.clear();
};

void StackerMeanValue::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};

void StackerMeanValue::add_photo_to_stack(unsigned int i_file)  {
    cout << "Adding " + m_files_to_stack[i_file] + " to stack\n";

    const string file_address = m_files_to_stack[i_file];
    const bool apply_alignment = m_apply_alignment[i_file];
    const FileAlignmentInformation alignment_info = apply_alignment ? m_photo_alignment_handler->get_alignment_parameters(file_address) : FileAlignmentInformation();
    const float shift_x         = alignment_info.shift_x;
    const float shift_y         = alignment_info.shift_y;
    const float rot_center_x    = alignment_info.rotation_center_x;
    const float rot_center_y    = alignment_info.rotation_center_y;
    const float rotation        = alignment_info.rotation;

    CalibratedPhotoHandler calibrated_photo(file_address);
    calibrated_photo.define_alignment(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    if (m_flat_frame_handler != nullptr) {
        calibrated_photo.register_flat_frame(m_flat_frame_handler.get());
    }
    if (m_hot_pixel_identifier != nullptr) {
        calibrated_photo.register_hot_pixel_identifier(m_hot_pixel_identifier.get());
    }
    calibrated_photo.calibrate();


    for (unsigned int i_thread = 0; i_thread < m_mutexes.size(); i_thread++) {
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
    }

    m_n_tasks_processed++;
};

int StackerMeanValue::get_tasks_total() const  {
    return m_files_to_stack.size();
};