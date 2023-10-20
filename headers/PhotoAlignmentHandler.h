#pragma once

#include "../headers/ReferencePhotoHandler.h"

#include <memory>
#include <string>
#include <vector>

namespace   AstroPhotoStacker   {
    struct FileAlignmentInformation    {
        std::string file_address;
        float shift_x;
        float shift_y;
        float rotation_center_x;
        float rotation_center_y;
        float rotation;
        float ranking;
    };
    class PhotoAlignmentHandler    {
        public:
            PhotoAlignmentHandler() = default;

            void read_from_text_file(const std::string &alignment_file_address);

            void save_to_text_file(const std::string &alignment_file_address);

            void align_files(const std::string &reference_file_address, const std::vector<std::string> &files);

            void align_all_files_in_folder(const std::string &reference_file_address, const std::string &raw_files_folder);

            void reset();

            void set_number_of_cpu_threads(unsigned int n_cpu)  { m_n_cpu = n_cpu; }

            FileAlignmentInformation get_alignment_parameters(const std::string &file_address) const;

            std::vector<std::string> get_file_addresses() const;

            void limit_number_of_files(int n_files);

            void limit_fraction_of_files(float fraction);

        private:
            std::string m_reference_file_address = "";
            std::vector<FileAlignmentInformation>   m_alignment_information_vector;

            unsigned int m_n_cpu = 1;

            std::unique_ptr<ReferencePhotoHandler> m_reference_photo_handler = nullptr;

            const std::string c_reference_file_header = "reference_file";

    };
}