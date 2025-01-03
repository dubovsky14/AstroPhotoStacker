#include "../headers/StackerSimpleBase.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/CustomScopeMutex.h"

#include "../headers/thread_pool.h"

#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

StackerSimpleBase::StackerSimpleBase(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerBase(number_of_colors, width, height, interpolate_colors),
    m_number_of_stacked_pixels(number_of_colors, vector<unsigned short>(width*height, 0))   {
};

void StackerSimpleBase::calculate_stacked_photo_internal()  {
    m_mutexes = vector<mutex>(m_n_cpu);

    const int height_range = get_height_range_limit();

    allocate_arrays_for_stacking(height_range);

    for (int y_min = 0; y_min < m_height; y_min += height_range) {
        if (y_min != 0) {
            reset_values_in_arrays_for_stacking();
        }

        const int y_max = min(y_min + height_range, m_height);
        if (m_n_cpu == 1)   {
            for (unsigned int i_file = 0; i_file < m_frames_to_stack.size(); i_file++) {
                add_photo_to_stack(i_file, y_min, y_max);
            }
        }
        else    {
            thread_pool pool(m_n_cpu);
            for (unsigned int i_file = 0; i_file < m_frames_to_stack.size(); i_file++) {
                pool.submit([this, i_file, y_min, y_max]() {
                    add_photo_to_stack(i_file, y_min, y_max);
                });
            }
            pool.wait_for_tasks();
        }
        calculate_final_image(y_min, y_max);
    }

    fix_empty_pixels();

    deallocate_arrays_for_stacking();
};

void StackerSimpleBase::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};


void StackerSimpleBase::add_photo_to_stack(unsigned int i_file, int y_min, int y_max)  {
    cout << "Adding " + m_frames_to_stack[i_file].to_string() + " to stack\n";
    CalibratedPhotoHandler calibrated_photo = get_calibrated_photo(i_file, y_min, y_max);

    const int pixel_shift = y_min*m_width;

    unsigned int i_thread = 0;
    while (true) {
        i_thread = (i_thread+1) % m_mutexes.size();
        CustomScopeMutex scope_mutex(&m_mutexes[i_thread]);
        if (!scope_mutex.is_locked()) {
            continue;
        }

        if (m_interpolate_colors)   {
            for (int color = 0; color < 3; color++)   {
                for (int y = y_min; y < y_max; y++)  {
                    for (int x = 0; x < m_width; x++)   {
                        const int index = y*m_width + x;
                        const auto value = calibrated_photo.get_value_by_reference_frame_index(index, color);
                        process_pixel(color, index - pixel_shift, value, i_thread);
                    }
                }
            }
        }
        else   {
            short int value;
            char color;
            for (int y = y_min; y < y_max; y++)  {
                for (int x = 0; x < m_width; x++)   {
                    calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
                    if (color >= 0) {
                        process_pixel(color, (y-y_min)*m_width + x, value, i_thread);
                    }
                }
            }
        }
        break;
    }

    m_n_tasks_processed++;
};

int StackerSimpleBase::get_height_range_limit() const  {
    return m_height;
};

int StackerSimpleBase::get_tasks_total() const  {
    const long long int n_files = m_frames_to_stack.size();
    const int height_range = get_height_range_limit();
    int n_slices = m_height/height_range + (m_height % height_range > 0);

    return n_slices*n_files;
};

unsigned long long StackerSimpleBase::get_maximal_memory_usage(int number_of_frames) const {
    const unsigned long long resolution = m_width*m_height;
    const unsigned long long stacked_image_size = m_number_of_colors*sizeof(double)*resolution;

    return stacked_image_size;
};
