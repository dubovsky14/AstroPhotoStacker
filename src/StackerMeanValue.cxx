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


void StackerMeanValue::save_stacked_photo_as_png(const std::string &file_address) const {
    double max_value = 0;
    for (int color = 0; color < m_number_of_colors; color++) {
        for (int y = 0; y < m_height; y++)  {
            for (int x = 0; x < m_width; x++)   {
                const unsigned int index = y*m_width + x;
                if (m_stacked_image[color][index] > max_value) {
                    max_value = m_stacked_image[color][index];
                }
            }
        }
    }
    double scale_factor = 65534 / max_value;


    std::cout << "save_stacked_photo_as_png:\n";
    std::cout << "width: " << m_width << "\n";
    std::cout << "height: " << m_height << "\n";
    std::cout << "image_settings: " << CV_16UC3 << "\n\n";
        std::cout << "sizeof(cv::Vec3w): " << sizeof(cv::Vec3w) << "\n\n";

    cv::Mat image(m_height, m_width, CV_16UC3);
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            cv::Vec3w& pixel = image.at<cv::Vec3w>(y, x);
            const unsigned int index = y*m_width + x;
            pixel[0] = scale_factor*m_stacked_image[0][index];
            pixel[1] = scale_factor*m_stacked_image[1][index];
            pixel[2] = scale_factor*m_stacked_image[2][index];
        }
    }
    cv::imwrite(file_address, image);
};