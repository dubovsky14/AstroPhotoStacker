#pragma once

#include "../headers/Metadata.h"

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <array>

namespace AstroPhotoStacker {

    /**
     * @brief Class for writing SER video files (astronomical video format)
     */
    class VideoWriterSER {
        public:
            VideoWriterSER() = delete;

            VideoWriterSER(const std::string &file_address, const Metadata &first_frame_metadata, unsigned int width, unsigned int height, float fps, unsigned int bit_depth);

            void write_frame(const std::vector<unsigned short>  &frame_data);

            void write_frame(const std::vector<short>           &frame_data);

            void write_frame(const std::vector<unsigned char>   &frame_data);

            void save_file();

            static int get_int_code_for_bayer_matrix(const std::array<char, 4> &bayer_matrix);

            static int get_int_code_for_bayer_matrix(const std::string &bayer_matrix);

            static unsigned long long int unix_time_to_microsoft_time(unsigned int unix_time);

            ~VideoWriterSER();

        private:

            void write_metadata();

            void update_frame_count_in_header();

            std::string get_instrument_string();

            std::string get_telescope_string();


            std::string m_file_address;
            Metadata m_first_frame_metadata;
            std::unique_ptr<std::ofstream> m_file = nullptr;
            unsigned int m_frame_count = 0;
            unsigned int m_width;
            unsigned int m_height;
            unsigned int m_bit_depth;
            float m_fps = 1;

    };
}