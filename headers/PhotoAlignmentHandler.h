#pragma once

#include "../headers/ReferencePhotoHandler.h"

#include <memory>
#include <string>
#include <vector>

namespace   AstroPhotoStacker   {
    class PhotoAlignmentHandler    {
        public:
            PhotoAlignmentHandler() = default;

            void read_from_text_file(const std::string &alignment_file_address);

            void save_to_text_file(const std::string &alignment_file_address);

            void align_files(const std::string &reference_file_address, const std::vector<std::string> &files);

            void align_all_files_in_folder(const std::string &reference_file_address, const std::string &raw_files_folder);

            void reset();

            void set_number_of_cpu_threads(unsigned int n_cpu)  { m_n_cpu = n_cpu; }

            void get_alignment_parameters(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const;

            std::vector<std::string> get_file_addresses() const { return m_file_addresses; };

        private:
            std::string m_reference_file_address = "";
            std::vector<std::string> m_file_addresses;
            std::vector<float> m_shift_x;
            std::vector<float> m_shift_y;
            std::vector<float> m_rotation_center_x;
            std::vector<float> m_rotation_center_y;
            std::vector<float> m_rotation;
            std::vector<float> m_ranking;

            unsigned int m_n_cpu = 1;

            std::unique_ptr<ReferencePhotoHandler> m_reference_photo_handler = nullptr;

            const std::string c_reference_file_header = "reference_file";

    };
}