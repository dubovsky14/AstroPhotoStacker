#pragma once


#include <string>
#include <fstream>
#include <memory>
#include <map>

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

            const unsigned short int *get_data() const {
                return m_data.get();
            }


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
            float m_exposure_time;

            std::unique_ptr<unsigned short int[]> m_data;
    };
}