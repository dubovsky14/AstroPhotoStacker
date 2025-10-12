#pragma once

#include "../headers/RawFileReaderBase.h"

#include <string>
#include <fstream>
#include <map>

namespace AstroPhotoStacker {

    bool is_fit_file(const std::string &file_address);

    class RawFileReaderFit : public RawFileReaderBase {
        public:
            RawFileReaderFit(const InputFrame &input_frame);

            virtual std::vector<PixelType> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) override;

            virtual void get_photo_resolution(int *width, int *height) override;

        protected:
            virtual Metadata read_metadata_without_cache() override;

        private:
            std::vector<PixelType> m_data;
            Metadata m_metadata;

            std::map<std::string, std::string> m_metadata_map;


            int m_width;
            int m_height;
            int m_bit_depth = 16;
            unsigned int m_zero_point;
            std::array<char, 4> m_bayer_matrix = {-1,-1,-1,-1};

            void read_data(std::ifstream *file);
            void read_data_8bit(std::ifstream *file);
            void read_data_16bit(std::ifstream *file);

            void read_header(std::ifstream *file);

            void parse_header(const std::string &header);

            std::string get_header_string(std::ifstream *file);

            void fill_metadata();

            void process_bayer_matrix(const std::string &bayer_matrix);

            static int get_unix_timestamp(const std::string &time_string);
    };
}