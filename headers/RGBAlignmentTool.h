#pragma once

#include "../headers/ImageFilesInputOutput.h"

#include <vector>
#include <string>

namespace AstroPhotoStacker {

    /**
     * @brief A class used to align RGB images for planetary images - to correct for atmospheric dispersion
     */
    template <typename PixelType = unsigned int>
    class RGBAlignmentTool  {
        public:
            RGBAlignmentTool() = delete;

            RGBAlignmentTool(const std::vector<std::vector<PixelType>> &original_image, unsigned int width, unsigned height)    {
                m_data_original = original_image;
                m_data_shifted = std::vector<std::vector<PixelType>>(3, std::vector<PixelType>(width*height, 0));
                m_width = width;
                m_height = height;
            };

            RGBAlignmentTool(const std::string &file_address)   {
                m_data_original = read_rgb_image<PixelType>(InputFrame(file_address), &m_width, &m_height);
                m_data_shifted = std::vector<std::vector<PixelType>>(3, std::vector<PixelType>(m_width*m_height, 0));
            };

            void calculate_shifted_image(const std::pair<int,int> &shift_red, const std::pair<int,int> &shift_blue) {
                m_data_shifted = std::vector<std::vector<PixelType>>(3, std::vector<PixelType>(m_width*m_height, 0));
                for (int x = 0; x < m_width; x++) {
                    const int x_shifted_red = x + shift_red.first;
                    const int x_shifted_blue = x + shift_blue.first;
                    for (int y = 0; y < m_height; y++) {
                        const int y_shifted_red = y + shift_red.second;
                        const int y_shifted_blue = y + shift_blue.second;

                        if (x_shifted_red >= 0 && x_shifted_red < m_width && y_shifted_red >= 0 && y_shifted_red < m_height) {
                            m_data_shifted[0][y*m_width + x] = m_data_original[0][y_shifted_red*m_width + x_shifted_red];
                        }

                        if (x_shifted_blue >= 0 && x_shifted_blue < m_width && y_shifted_blue >= 0 && y_shifted_blue < m_height) {
                            m_data_shifted[2][y*m_width + x] = m_data_original[2][y_shifted_blue*m_width + x_shifted_blue];
                        }

                        m_data_shifted[1][y*m_width + x] = m_data_original[1][y*m_width + x];
                    }
                }
            };

            const std::vector<std::vector<PixelType>> &get_original_image() const   {
                return m_data_original;
            }

            const std::vector<std::vector<PixelType>> &get_shifted_image() const    {
                return m_data_shifted;
            };

            int get_width() const  {
                return m_width;
            };

            int get_height() const {
                return m_height;
            };

            void save_shifted_image(const std::string &file_address) const  {
                create_color_image(m_data_shifted[0].data(), m_data_shifted[1].data(), m_data_shifted[2].data(), m_width, m_height, file_address, CV_16UC3);
            };

        private:
            std::vector<std::vector<PixelType>> m_data_original;
            std::vector<std::vector<PixelType>> m_data_shifted;

            int m_width;
            int m_height;
    };
}