#include "../headers/StackerMedian.h"
#include "../headers/CalibratedPhotoHandler.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

StackerMedian::StackerMedian(int number_of_colors, int width, int height) :
    StackerBase(number_of_colors, width, height)   {
};

void StackerMedian::calculate_stacked_photo()  {
    const long long int n_files = m_files_to_stack.size();
    const int height_range = get_height_range_limit();

    if (height_range == 0) {
        throw runtime_error("The memory set by the user is not sufficient, please increase it");
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        m_values_to_stack.push_back(std::move(make_unique<short[]>(m_width*height_range*n_files)));
    }

    int i_slice = 0;
    int n_slices = m_height/height_range + (m_height % height_range > 0);
    for (int y_min = 0; y_min < m_height; y_min += height_range) {

        i_slice++;
        const int y_max = min(y_min + height_range, m_height);
        cout << "Stacking slice " << i_slice << " of " << n_slices << endl;
        // set all values to -1
        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (long long i_pixel = 0; i_pixel < m_width*height_range*n_files; i_pixel++) {
                m_values_to_stack[i_color][i_pixel] = -1;
            }
        }

        auto submit_photo_stack = [this, y_min, y_max](unsigned int file_index) {
            cout << string("Adding ") + m_files_to_stack[file_index] + string(" to stack\n");
            add_photo_to_stack(file_index, y_min, y_max);
        };

        thread_pool pool(m_n_cpu);
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            if (m_n_cpu > 1) {
                pool.submit(submit_photo_stack, i_file);
            }
            else {
                submit_photo_stack(i_file);
            }
        }
        pool.wait_for_tasks();

        auto submit_median_calculation = [this, y_min, y_max](int i_color, int y_final_array, int y_values_to_stack_array ) {
            process_line(y_final_array, y_values_to_stack_array, i_color);
        };

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int y_final_array = y_min; y_final_array < y_max; y_final_array++) {
                const int y_values_to_stack_array = y_final_array - y_min;
                if (m_n_cpu > 1) {
                    pool.submit(submit_median_calculation, i_color, y_final_array, y_values_to_stack_array);
                }
                else {
                    submit_median_calculation(i_color, y_final_array, y_values_to_stack_array);
                }
            }
        }
        pool.wait_for_tasks();
    }


    m_values_to_stack.clear();

    // fix green pixels
    if (m_number_of_colors == 3) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            m_stacked_image[1][i_pixel] /= 2;
        }
    }
    fix_empty_pixels();
};

void StackerMedian::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};

void StackerMedian::add_photo_to_stack(unsigned int file_index, int y_min, int y_max)  {
    const string &file_address = m_files_to_stack[file_index];
    const unsigned long long int n_files = m_files_to_stack.size();
    const bool apply_alignment = m_apply_alignment[file_index];

    const FileAlignmentInformation alignment_info = apply_alignment ? m_photo_alignment_handler->get_alignment_parameters(file_address) : FileAlignmentInformation();
    const float shift_x         = alignment_info.shift_x;
    const float shift_y         = alignment_info.shift_y;
    const float rot_center_x    = alignment_info.rotation_center_x;
    const float rot_center_y    = alignment_info.rotation_center_y;
    const float rotation        = alignment_info.rotation;

    CalibratedPhotoHandler calibrated_photo(file_address);
    calibrated_photo.define_alignment(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    calibrated_photo.limit_y_range(y_min, y_max);
    if (m_flat_frame_handler != nullptr) {
        calibrated_photo.register_flat_frame(m_flat_frame_handler.get());
    }
    if (m_hot_pixel_identifier != nullptr) {
        calibrated_photo.register_hot_pixel_identifier(m_hot_pixel_identifier.get());
    }
    calibrated_photo.calibrate();

    unsigned int value;
    char color;
    for (int y = y_min; y < y_max; y++)  {
        for (int x = 0; x < m_width; x++)   {
            calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
            const unsigned int index_stacking_array = (y-y_min)*m_width + x;
            if (color >= 0) {
                m_values_to_stack[color][n_files*index_stacking_array + file_index] = value;
            }
        }
    }
};


int StackerMedian::get_height_range_limit() const {
    int height_range = m_height;
    const long long int n_files = m_files_to_stack.size();
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - 10ULL*m_width*m_height;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_width*n_files*sizeof(unsigned short);
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};


void StackerMedian::process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)    {
    const long long int n_files = m_files_to_stack.size();
    for (int i_width = 0; i_width < m_width; i_width++) {
        const unsigned long long int pixel_index = m_width*y_index_final_array + i_width;
        const unsigned long long int pixel_index_stacking_array = m_width*y_index_values_to_stack_array*n_files + i_width*n_files;

        short *slice_begin = &m_values_to_stack[i_color][pixel_index_stacking_array];
        short *slice_end   = &m_values_to_stack[i_color][pixel_index_stacking_array + n_files];

        int number_of_stacked_pixels = 0;
        for (int i = 0; i < n_files; i++) {
            if (slice_begin[i] >= 0) {
                number_of_stacked_pixels++;
            }
        }
        int first_non_negative_index = n_files - number_of_stacked_pixels;

        sort(slice_begin, slice_end);

        m_stacked_image[i_color][pixel_index] = get_stacked_value_from_pixel_array(slice_begin + first_non_negative_index, number_of_stacked_pixels);
    }
};


double StackerMedian::get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }
    else if (number_of_stacked_pixels % 2 == 0) {
        return (ordered_array_begin[number_of_stacked_pixels/2] + ordered_array_begin[number_of_stacked_pixels/2 - 1])/2;
    }
    else {
        return ordered_array_begin[number_of_stacked_pixels/2];
    }
};