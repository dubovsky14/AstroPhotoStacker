#pragma once

#include <string>
#include <fstream>
#include <array>

namespace AstroPhotoStacker {
    class ZWOVideoTextFileInfo {
        public:
            ZWOVideoTextFileInfo(const std::string &input_file);

            int get_gain() const {
                return m_gain;
            }

            float get_exposure_time() const {
                return m_exposure_time;
            }

            std::array<char, 4> get_bayer_matrix() const {
                return m_bayer_matrix;
            }

            static bool is_valid_zwo_video_text_file(const std::string &file_address) {
                std::ifstream file(file_address);
                if (!file.is_open()) {
                    return false;
                }

                ZWOVideoTextFileInfo info(file_address);
                return info.get_gain() != -1 && info.get_exposure_time() != 0 && info.get_bayer_matrix() != std::array<char, 4>{-1,-1,-1,-1};
            }

        private:
            void read_data(std::ifstream *file);

            void read_bayer_matrix(const std::string &matrix_string);

            int m_gain = -1;
            float m_exposure_time = 0;
            std::array<char, 4> m_bayer_matrix = {-1,-1,-1,-1};
    };
}