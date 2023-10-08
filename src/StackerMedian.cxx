#include "../headers/StackerMedian.h"
#include "../headers/CalibratedPhotoHandler.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

StackerMedian::StackerMedian(int number_of_colors, int width, int height) :
    StackerBase(number_of_colors, width, height)   {
};

void StackerMedian::calculate_stacked_photo()  {
    const unsigned int n_files = m_files_to_stack.size();
    const int height_range = get_height_range_limit();

    if (height_range == 0) {
        throw runtime_error("The memory set by the user is not sufficient, please increase it");
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        m_values_to_stack.push_back(std::move(make_unique<unsigned short[]>(m_width*height_range*n_files)));
        m_number_of_stacked_pixels.push_back(vector<unsigned short>(m_width*m_height, 0));
    }

    int i_slice = 0;
    int n_slices = m_height/height_range + (m_height % height_range > 0);
    for (int y_min = 0; y_min < m_height; y_min += height_range) {
        i_slice++;
        const int y_max = min(y_min + height_range, m_height);
        cout << "Stacking slice " << i_slice << " of " << n_slices << endl;
        for (const string &file_address : m_files_to_stack) {
            cout << "Adding " << file_address << " to stack" << endl;
            add_photo_to_stack(file_address, y_min, y_max);
        }

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int i_pixel = m_width*y_min; i_pixel < m_width*y_max; i_pixel++) {
                const unsigned long long int pixel_index = n_files*i_pixel;
                const unsigned int number_of_stacked_pixels = m_number_of_stacked_pixels[i_color][i_pixel];
                const unsigned long long int pixel_index_stacking_array = pixel_index - n_files*m_width*y_min;
                sort(&m_values_to_stack[i_color][pixel_index_stacking_array], &m_values_to_stack[i_color][pixel_index_stacking_array+number_of_stacked_pixels]);

                if (number_of_stacked_pixels == 0) {
                    m_stacked_image[i_color][i_pixel] = 0;
                }
                else if (number_of_stacked_pixels % 2 == 0) {
                    m_stacked_image[i_color][i_pixel] = (m_values_to_stack[i_color][pixel_index_stacking_array + number_of_stacked_pixels/2] + m_values_to_stack[i_color][pixel_index_stacking_array + number_of_stacked_pixels/2 - 1])/2;
                }
                else {
                    m_stacked_image[i_color][i_pixel] = m_values_to_stack[i_color][pixel_index_stacking_array + number_of_stacked_pixels/2];
                }
            }
        }
    }


    m_values_to_stack.clear();
    m_number_of_stacked_pixels.clear();

    // fix green pixels
    if (m_number_of_colors == 3) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            m_stacked_image[1][i_pixel] /= 2;
        }
    }
};


void StackerMedian::add_photo_to_stack(const std::string &file_address, int y_min, int y_max)  {
    const unsigned int n_files = m_files_to_stack.size();
    float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
    m_photo_alignment_handler->get_alignment_parameters(file_address, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation);

    CalibratedPhotoHandler calibrated_photo(file_address);
    calibrated_photo.define_alignment(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    calibrated_photo.limit_y_range(y_min, y_max);
    if (m_flat_frame_handler != nullptr) {
        calibrated_photo.register_flat_frame(m_flat_frame_handler.get());
    }
    calibrated_photo.calibrate();

    unsigned int value;
    char color;
    for (int y = y_min; y < y_max; y++)  {
        for (int x = 0; x < m_width; x++)   {
            calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
            if (color >= 0) {
                const unsigned int index = y*m_width + x;
                const unsigned int index_stacking_array = (y-y_min)*m_width + x;
                m_values_to_stack[color][n_files*index_stacking_array + m_number_of_stacked_pixels[color][index]] = value;
                m_number_of_stacked_pixels[color][index] += 1;
            }
        }
    }
};


int StackerMedian::get_height_range_limit() const {
    int height_range = m_height;
    const int n_files = m_files_to_stack.size();
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - 10ULL*m_width*m_height;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_width*n_files*sizeof(unsigned short);
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};