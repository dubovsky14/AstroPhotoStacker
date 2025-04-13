#pragma once

#include "../headers/InputFrame.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"

#include "../headers/Debayring.h"

namespace AstroPhotoStacker {

    /**
     * @brief Class for reading and storing data from input image files - raw files, RGB files, video frames.
     */
    template<typename PixelType = short int>
    class InputFrameData    {
        public:
            InputFrameData() = delete;

            InputFrameData(const InputFrame &input_frame)   {
                m_is_raw_file = AstroPhotoStacker::is_raw_file(input_frame.get_file_address());
                if (m_is_raw_file)  {
                    m_image_data_raw = read_raw_file<PixelType>(input_frame, &m_width, &m_height, &m_colors);
                    m_n_colors = *std::max_element(m_colors.begin(), m_colors.end()) + 1;
                }
                else {
                    m_image_data_color = read_rgb_image<PixelType>(input_frame, &m_width, &m_height);
                    m_n_colors = m_image_data_color.size();
                }
            };

            const InputFrame& get_input_frame() const   {
                return m_input_frame;
            };

            int get_width() const   {
                return m_width;
            };

            int get_height() const  {
                return m_height;
            };

            bool is_raw_file() const    {
                return m_is_raw_file;
            };

            char get_color(int x, int y) const  {
                return m_colors[x + y * m_width];
            };

            char get_color(int i_pixel) const   {
                return m_colors[i_pixel];
            };

            PixelType &get_pixel_value_raw(int index)   {
                return m_image_data_raw[index];
            };

            PixelType &get_pixel_value_raw(int x, int y)   {
                int index = x + y * m_width;
                return get_pixel_value_raw(index);
            };

            const PixelType &get_pixel_value_raw(int index) const   {
                return m_image_data_raw[index];
            };

            const PixelType &get_pixel_value_raw(int x, int y) const   {
                int index = x + y * m_width;
                return get_pixel_value_raw(index);
            };

            PixelType get_pixel_value(int x, int y, char color) const   {
                if (!is_raw_file())  {
                    return m_image_data_color[color][x + y * m_width];
                }
                else {
                    return get_color(x,y) == color ? m_image_data_raw[x + y * m_width] : -1;
                }
            };

            PixelType *get_pixel_value_pointer(int x, int y, char color)    {
                if (!is_raw_file())  {
                    return &m_image_data_color[color][x + y * m_width];
                }
                else {
                    return get_color(x,y) == color ? &m_image_data_raw[x + y * m_width] : nullptr;
                }
            }

            const std::vector<std::vector<PixelType>>& get_image_data_color() const {
                return m_image_data_color;
            };

            const std::vector<PixelType>& get_image_data_raw() const    {
                return m_image_data_raw;
            };

            void debayer()  {
                if (!is_raw_file())  {
                    throw std::runtime_error("Attempted to debayer a non-raw file.");
                }

                m_image_data_color = debayer_raw_data(m_image_data_raw, m_width, m_height, m_colors);

                m_is_raw_file = false;
                m_image_data_raw.clear();
                m_colors.clear();
            };

            int get_number_of_colors() const {
                return m_image_data_color.size();
            };

        private:
            InputFrame m_input_frame;
            int m_width;
            int m_height;
            std::vector<std::vector<PixelType>> m_image_data_color;
            std::vector<PixelType> m_image_data_raw;
            std::vector<char> m_colors;
            bool m_is_raw_file;
            int m_n_colors = 1;
    };
}