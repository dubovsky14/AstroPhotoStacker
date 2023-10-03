#include "../headers/StackerBase.h"

#include <opencv2/opencv.hpp>

using namespace std;
using namespace AstroPhotoStacker;


StackerBase::StackerBase(int number_of_colors, int width, int height)   {
    m_number_of_colors = number_of_colors;
    m_width = width;
    m_height = height;
    m_stacked_image = vector<vector<unsigned int>>(m_number_of_colors, vector<unsigned int>(m_width*m_height, 0));
    m_number_of_stacked_pixels = vector<vector<unsigned short>>(m_number_of_colors, vector<unsigned short>(m_width*m_height, 0));
};

void StackerBase::add_photo(const ShiftedPhotoHandler &photo)    {
    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            unsigned int value;
            char color;
            photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
            if (color >= 0) {
                const unsigned int index = y*m_width + x;
                m_stacked_image[color][index]   += value;
                m_number_of_stacked_pixels[color][index] += 1;
            }
        }
    }
};

void StackerBase::calculate_stacked_photo() {
    m_stacked_image_double = vector<vector<double>>(m_number_of_colors, vector<double>(m_width*m_height, 0));
    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            for (int color = 0; color < m_number_of_colors; color++) {
                const unsigned int index = y*m_width + x;
                if (m_number_of_stacked_pixels[color][index] > 0) {
                    m_stacked_image_double[color][index] = m_stacked_image[color][index] / double (m_number_of_stacked_pixels[color][index]);
                }
                else {
                    m_stacked_image_double[color][index] = 0;
                }
            }
        }
    }

    // fix green pixels
    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            const unsigned int index = y*m_width + x;
            m_stacked_image_double[1][index] /= 2;
        }
    }
};


void StackerBase::save_stacked_photo_as_png(const std::string &file_address) const {
    cv::Mat image(m_height, m_width, CV_16UC3);
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            cv::Vec3b& pixel = image.at<cv::Vec3b>(y, x);
            const unsigned int index = y*m_width + x;
            pixel[0] = m_stacked_image_double[0][index];
            pixel[1] = m_stacked_image_double[1][index];
            pixel[2] = m_stacked_image_double[2][index];
        }
    }
    cv::imwrite(file_address, image);
};