#pragma once

#include "../headers/ReferencePhotoHandler.h"

#include <memory>
#include <string>
#include <vector>

namespace   AstroPhotoStacker   {
    class PhotoAlignmentHandler    {
        public:
            void read_from_text_file(const std::string &alignment_file_address);

            void save_to_text_file(const std::string &alignment_file_address);

            void align_files(const std::string &reference_file_address, const std::vector<std::string> &files);

            void align_all_files_in_folder(const std::string &reference_file_address, const std::string &raw_files_folder);

            void reset();

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

            std::unique_ptr<ReferencePhotoHandler> m_reference_photo_handler = nullptr;

            const std::string c_reference_file_header = "reference_file";

    };
}