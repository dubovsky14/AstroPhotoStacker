#pragma once


#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <array>

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
                return m_is_rgb;
            };

            const std::vector<unsigned short int> &get_data() const {
                return m_data;
            }

            std::vector<char> get_colors() const;

        private:
            void read_header(std::ifstream &file);

            std::string get_header_string(std::ifstream &file);

            void parse_header(const std::string &header);

            void fill_metadata();

            void read_data(std::ifstream &file);

            std::map<std::string, std::string> m_metadata;

            std::array<char, 4> m_bayer_matrix = {0,1,1,2}; // RGGB

            int m_width;
            int m_height;
            bool m_is_rgb = false;
            int m_bit_depth;
            unsigned int m_zero_point;
            float m_exposure_time;

            std::vector<unsigned short int> m_data;

            void process_bayer_matrix(const std::string &bayer_matrix);

            inline char get_color(int x, int y) const {
                return m_bayer_matrix[(x%2) + 2*(y%2)];
            }

            void apply_green_correction();
    };
}