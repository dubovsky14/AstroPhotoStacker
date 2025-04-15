#pragma once

#include "../headers/Metadata.h"

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>

#include <iostream>
namespace AstroPhotoStacker {
    class FitFileSaver {
        public:
            FitFileSaver(int width, int height);

            void set_metadata(const Metadata &metadata) {
                m_metadata = metadata;
            };

            const Metadata &get_metadata() const    {
                return m_metadata;
            };

            void set_bits_per_pixel(int bits_per_pixel);

            int get_bits_per_pixel() const  {
                return m_bits_per_pixel;
            };

            std::map<std::string, std::string> &additional_header_data()    {
                return m_additional_header_data;
            }

            void save(const std::string &filename, const std::vector<unsigned short> &image)  const  {
                std::ofstream file(filename, std::ios::binary);
                if (!file) {
                    throw std::runtime_error("Could not open file for writing.");
                }
                std::string header_string = get_header_string();
                file.write(header_string.c_str(), header_string.length());

                dump_image_into_data_block(&file, image);
            };

        private:
            std::string get_header_string() const;

            std::string get_padded_value(const std::string &value)  const;

            std::string get_padded_comment(const std::string &comment)  const;

            std::string get_padded_key(const std::string &key)  const;

            std::string get_padded_text(const std::string &text, int padded_length, bool padding_on_left)  const;

            std::string get_header_line(const std::string &key, const std::string &value, const std::string &comment = "no comment")  const;

            void add_final_header_padding(std::string *header_string)  const;

            template<typename PixelType>
            void dump_image_into_data_block(std::ofstream *file, const std::vector<PixelType> &image)  const    {
                if (m_bits_per_pixel == 8) {
                    std::vector<char> buffer(image.size());
                    for (size_t i = 0; i < image.size(); ++i) {
                        buffer[i] = static_cast<char>(static_cast<unsigned short>(image[i]) - m_b_zero); // minus here, because plus would give signed int and it would overflow (undefined behavior)
                    }
                    file->write(buffer.data(), buffer.size());
                }
                else if (m_bits_per_pixel == 16) {
                    std::vector<unsigned short> buffer(image.size());
                    for (size_t i = 0; i < image.size(); ++i) {
                        unsigned short pixel_shifted = static_cast<unsigned short>(image[i]) + m_b_zero; // plus here, because minus would give signed int and it would overflow (undefined behavior)
                        pixel_shifted = (pixel_shifted << 8) | (pixel_shifted >> 8); // swap bytes
                        buffer[i] = pixel_shifted;
                    }
                    file->write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(unsigned short));
                }
                else {
                    throw std::runtime_error("Unsupported bits per pixel value.");
                }
            };

            int m_bits_per_pixel = 16;
            unsigned short int m_b_zero = 32768;
            int m_width = 0;
            int m_height = 0;

            Metadata m_metadata;
            std::map<std::string, std::string> m_additional_header_data;

    };
}