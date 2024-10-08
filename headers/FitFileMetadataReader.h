#pragma once

#include "../headers/Metadata.h"

#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <limits>

namespace AstroPhotoStacker {
    class FitFileMetadataReader {
        public:
            FitFileMetadataReader() = delete;

            FitFileMetadataReader(const std::string &input_file);

            int get_width() const {
                return m_width;
            }

            int get_height() const {
                return m_height;
            }

            int get_bit_depth() const {
                return m_bit_depth;
            }

            float get_exposure_time() const {
                return m_exposure_time;
            }

            bool is_rgb() const {
                return m_is_rgb;
            };

            std::vector<char> get_colors() const;

            const Metadata& get_metadata() const {
                return m_metadata_struct;
            };

        protected:
            void read_header(std::ifstream &file);

            std::string get_header_string(std::ifstream &file);

            void parse_header(const std::string &header);

            void fill_metadata();

            std::map<std::string, std::string> m_metadata;

            std::array<char, 4> m_bayer_matrix = {0,1,1,2}; // RGGB

            Metadata m_metadata_struct;

            int m_width;
            int m_height;
            bool m_is_rgb = false;
            int m_bit_depth;
            unsigned int m_zero_point;
            float m_exposure_time;

            void process_bayer_matrix(const std::string &bayer_matrix);

            inline char get_color(int x, int y) const {
                return m_bayer_matrix[(x%2) + 2*(y%2)];
            }

            std::unique_ptr<std::ifstream> m_input_stream = nullptr;
    };
}