#include "../headers/StackerMeanValue.h"
#include "../headers/CalibratedPhotoHandler.h"

#include <opencv2/opencv.hpp>

#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

StackerMeanValue::StackerMeanValue(int number_of_colors, int width, int height) :
    StackerBase(number_of_colors, width, height),
    m_number_of_stacked_pixels(number_of_colors, vector<unsigned short>(width*height, 0))   {
};

void StackerMeanValue::calculate_stacked_photo()  {
    for (const string &file_address : m_files_to_stack) {
        cout << "Adding " << file_address << " to stack" << endl;
        add_photo_to_stack(file_address);
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            if (m_number_of_stacked_pixels[i_color][i_pixel] > 0) {
                m_stacked_image[i_color][i_pixel] /= m_number_of_stacked_pixels[i_color][i_pixel];
            }
            else {
                m_stacked_image[i_color][i_pixel] = 0;
            }
        }
    }

    // fix green pixels
    if (m_number_of_colors == 3) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            m_stacked_image[1][i_pixel] /= 2;
        }
    }
};


void StackerMeanValue::add_photo_to_stack(const std::string &file_address)  {
    float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
    m_photo_alignment_handler->get_alignment_parameters(file_address, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation);

    CalibratedPhotoHandler calibrated_photo(file_address);
    calibrated_photo.define_alignment(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    if (m_flat_frame_handler != nullptr) {
        calibrated_photo.apply_flat_frame(*m_flat_frame_handler);
    }
    calibrated_photo.calibrate();

    unsigned int value;
    char color;
    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
            if (color >= 0) {
                const unsigned int index = y*m_width + x;
                m_stacked_image[color][index]   += value;
                m_number_of_stacked_pixels[color][index] += 1;
            }
        }
    }
};
