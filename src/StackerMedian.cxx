#include "../headers/StackerMedian.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/TaskScheduler.hxx"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

StackerMedian::StackerMedian(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerBase(number_of_colors, width, height, interpolate_colors)   {
};

void StackerMedian::calculate_stacked_photo_internal()  {
    const long long int n_files = m_frames_to_stack.size();
    const int height_range = get_height_range_limit();

    if (height_range == 0) {
        throw runtime_error("The memory set by the user is not sufficient, please increase it");
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        m_values_to_stack.push_back(vector<PixelType>(m_width*height_range*n_files, -1));
    }

    int i_slice = 0;
    int n_slices = m_height/height_range + (m_height % height_range > 0);
    for (int y_min = 0; y_min < m_height; y_min += height_range) {

        i_slice++;
        const int y_max = min(y_min + height_range, m_height);
        cout << "Stacking slice " << i_slice << " of " << n_slices << endl;

        auto submit_photo_stack = [this, y_min, y_max](unsigned int file_index) {
            cout << string("Adding ") + m_frames_to_stack[file_index].to_string() + string(" to stack\n");
            add_photo_to_stack(file_index, y_min, y_max);
            m_n_tasks_processed++;
        };

        TaskScheduler pool({size_t(m_n_cpu)});
        for (unsigned int i_file = 0; i_file < m_frames_to_stack.size(); i_file++) {
            if (m_n_cpu > 1) {
                pool.submit(submit_photo_stack, {1}, i_file);
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
                    pool.submit(submit_median_calculation, {1}, i_color, y_final_array, y_values_to_stack_array);
                }
                else {
                    submit_median_calculation(i_color, y_final_array, y_values_to_stack_array);
                }
            }
        }
        pool.wait_for_tasks();
    }


    m_values_to_stack.clear();

    fix_empty_pixels();
};

void StackerMedian::add_photo_to_stack(unsigned int file_index, int y_min, int y_max)  {
    const unsigned long long int n_files = m_frames_to_stack.size();

    CalibratedPhotoHandler calibrated_photo = get_calibrated_photo(file_index, y_min, y_max);

    for (int color = 0; color < 3; color++)   {
        const unsigned int slice_shift = y_min*m_width;
        for (int y = y_min; y < y_max; y++)  {
            const unsigned int start_of_line_index_reference_frame = y*m_width;
            const unsigned int start_of_line_index_this_slice = (y*m_width - slice_shift)*n_files + file_index;
            for (int x = 0; x < m_width; x++)   {
                m_values_to_stack[color][start_of_line_index_this_slice + x*n_files] = calibrated_photo.get_value_by_reference_frame_index(start_of_line_index_reference_frame+x, color);
            }
        }
    }
};


int StackerMedian::get_height_range_limit() const {
    int height_range = m_height;
    const long long int n_files = m_frames_to_stack.size();
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_needed_for_stacked_image = 3*sizeof(double)*m_width*m_height;
        const unsigned long long int memory_needed_for_calibrated_photos = m_n_cpu*3*sizeof(PixelType)*m_width*m_height;
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - memory_needed_for_stacked_image - memory_needed_for_calibrated_photos;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_width*n_files*sizeof(PixelType);
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};


void StackerMedian::process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)    {
    const long long int n_files = m_frames_to_stack.size();
    for (int i_width = 0; i_width < m_width; i_width++) {
        const unsigned long long int pixel_index = m_width*y_index_final_array + i_width;
        const unsigned long long int pixel_index_stacking_array = m_width*y_index_values_to_stack_array*n_files + i_width*n_files;

        PixelType *slice_begin = &m_values_to_stack[i_color][pixel_index_stacking_array];
        PixelType *slice_end   = &m_values_to_stack[i_color][pixel_index_stacking_array + n_files];

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


double StackerMedian::get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels) {
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

int StackerMedian::get_tasks_total() const  {
    const long long int n_files = m_frames_to_stack.size();
    const int height_range = get_height_range_limit();
    int n_slices = m_height/height_range + (m_height % height_range > 0);

    return n_slices*n_files;
};

unsigned long long StackerMedian::get_maximal_memory_usage(int number_of_frames) const {
    const unsigned long long resolution = m_width*m_height;
    const unsigned long long stacked_image_size = m_number_of_colors*sizeof(double)*resolution;
    const unsigned long long all_frames_data    = m_number_of_colors*number_of_frames*sizeof(PixelType)*resolution;

    const unsigned long long memory_usage_total = stacked_image_size + all_frames_data;

    return std::min<unsigned long long>(memory_usage_total, m_memory_usage_limit_in_mb*1024ULL*1024ULL);
};
