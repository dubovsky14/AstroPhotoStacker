#pragma once


#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <vector>

namespace AstroPhotoStacker {
    class FitFileReader {
        public:
            FitFileReader(const std::string &input_file);

            const std::map<std::string, std::string> &get_metadata() const {
                return m_metadata;
            }

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
                return m_colors.size() > 0;
            };

            const std::vector<unsigned short int> &get_data() const {
                return m_data;
            }

            const std::vector<char> &get_colors() const {
                return m_colors;
            };

        private:
            void read_header(std::ifstream &file);

            std::string get_header_string(std::ifstream &file);

            void parse_header(const std::string &header);

            void fill_metadata();

            void read_data(std::ifstream &file);

            std::map<std::string, std::string> m_metadata;

            int m_width;
            int m_height;
            int m_bit_depth;
            unsigned int m_zero_point;
            float m_exposure_time;

            std::vector<unsigned short int> m_data;
            std::vector<char> m_colors;

    };
}