#include "../headers/StackerSimpleBase.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/CustomScopeMutex.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

StackerSimpleBase::StackerSimpleBase(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerBase(number_of_colors, width, height, interpolate_colors),
    m_number_of_stacked_pixels(number_of_colors, vector<unsigned short>(width*height, 0))   {
};

void StackerSimpleBase::calculate_stacked_photo()  {
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

void StackerSimpleBase::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};


void StackerSimpleBase::add_photo_to_stack(unsigned int i_file, int y_min, int y_max)  {
    cout << "Adding " + m_files_to_stack[i_file] + " to stack\n";
    CalibratedPhotoHandler calibrated_photo = get_calibrated_photo(i_file, y_min, y_max);

    unsigned int i_thread = 0;
    while (true) {
        i_thread = (i_thread+1) % m_mutexes.size();
        CustomScopeMutex scope_mutex(&m_mutexes[i_thread]);
        if (!scope_mutex.is_locked()) {
            continue;
        }

        if (m_interpolate_colors)   {
            for (int color = 0; color < 3; color++)   {
                for (int y = 0; y < m_height; y++)  {
                    for (int x = 0; x < m_width; x++)   {
                        const int index = y*m_width + x;
                        const auto value = calibrated_photo.get_value_by_reference_frame_index(index, color);
                        process_pixel(color, index, value, i_thread);
                    }
                }
            }
        }
        else   {
            unsigned int value;
            char color;
            for (int y = 0; y < m_height; y++)  {
                for (int x = 0; x < m_width; x++)   {
                    calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
                    if (color >= 0) {
                        process_pixel(color, y*m_width + x, value, i_thread);
                    }
                }
            }
        }
        break;
    }

    m_n_tasks_processed++;
};

int StackerSimpleBase::get_height_range_limit() const  {
    return m_width;
};

int StackerSimpleBase::get_tasks_total() const  {
    return m_files_to_stack.size();
};