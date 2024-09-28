#pragma once

#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/LocalShiftsHandler.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <tuple>
#include <optional>

namespace   AstroPhotoStacker   {
    struct FileAlignmentInformation    {
        std::string file_address = "";
        float shift_x            = 0;
        float shift_y            = 0;
        float rotation_center_x  = 0;
        float rotation_center_y  = 0;
        float rotation           = 0;
        float ranking            = 0;
        LocalShiftsHandler local_shifts_handler;
    };

    /**
     * @class PhotoAlignmentHandler
     * @brief Handles the alignment info for individual photos.
     *
     * The PhotoAlignmentHandler class provides methods to add alignment information, read from and save to a text file,
     * align raw based on a reference file.
     */
    class PhotoAlignmentHandler {
        public:
            /**
             * @brief Default constructor for the PhotoAlignmentHandler class.
             */
            PhotoAlignmentHandler() = default;

            /**
             * @brief Adds alignment information for a file.
             * @param file_address The address of the raw file.
             * @param x_shift The horizontal shift.
             * @param y_shift The vertical shift.
             * @param rotation_center_x The x-coordinate of the rotation center.
             * @param rotation_center_y The y-coordinate of the rotation center.
             * @param rotation The rotation angle.
             * @param ranking The ranking of the alignment.
             */
            void add_alignment_info(const std::string& file_address, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking, const LocalShiftsHandler &local_shifts_handler = LocalShiftsHandler());

            /**
             * @brief Reads alignment information from a text file.
             * @param alignment_file_address The address of the alignment file.
             */
            void read_from_text_file(const std::string& alignment_file_address);

            /**
             * @brief Saves alignment information to a text file.
             * @param alignment_file_address The address of the alignment file.
             */
            void save_to_text_file(const std::string& alignment_file_address);

            /**
             * @brief Aligns files based on a reference file.
             * @param reference_file_address The address of the reference file.
             * @param files The vector of file addresses to align.
             */
            void align_files(const std::string& reference_file_address, const std::vector<std::string>& files);

            /**
             * @brief Aligns all files in a folder based on a reference file.
             * @param reference_file_address The address of the reference file.
             * @param raw_files_folder The address of the folder containing the raw files.
             */
            void align_all_files_in_folder(const std::string& reference_file_address, const std::string& raw_files_folder);

            /**
             * @brief Resets the alignment parameters.
             */
            void reset();

            /**
             * @brief Sets the number of CPU threads to use for alignment.
             * @param n_cpu The number of CPU threads.
             */
            void set_number_of_cpu_threads(unsigned int n_cpu) {m_n_cpu = n_cpu;};

            /**
             * @brief Gets the alignment parameters for a specific file.
             * @param file_address The address of the file.
             * @return The alignment parameters for the file.
             */
            FileAlignmentInformation get_alignment_parameters(const std::string& file_address) const;

            /**
             * @brief Gets the vector of alignment parameters for all files.
             * @return The vector of alignment parameters.
             */
            const std::vector<FileAlignmentInformation>& get_alignment_parameters_vector() const;

            /**
             * @brief Gets the addresses of all files.
             * @return The vector of file addresses.
             */
            std::vector<std::string> get_file_addresses() const;

            /**
             * @brief Limits the number of files to align.
             * @param n_files The maximum number of files to align.
             */
            void limit_number_of_files(int n_files);

            /**
             * @brief Limits the fraction of files to align.
             * @param fraction The fraction of files to align (between 0 and 1).
             */
            void limit_fraction_of_files(float fraction);

            /**
             * @brief Gets the number of aligned files.
             * @return The number of aligned files as an atomic integer.
             */
            const std::atomic<int>& get_number_of_aligned_files() const;

            /**
             * @brief Sets the alignment method.
             * @param alignment_method The alignment method.
             */
            void set_alignment_method(const std::string& alignment_method) {m_alignment_method = alignment_method;};

            /**
             * @brief Gets the alignment method.
             * @return The alignment method.
             */
            const std::string& get_alignment_method() const {return m_alignment_method;};

            std::vector<std::tuple<int,int,int,int,bool>> get_local_shifts(const std::string& file_address) const;

            const std::vector<std::vector<std::tuple<int,int,int,int,bool>>>& get_local_shifts_vector() const {return m_local_shifts_vector;};

        private:
            std::string m_reference_file_address = "";
            std::vector<FileAlignmentInformation> m_alignment_information_vector;
            std::vector<std::vector<std::tuple<int,int,int,int,bool>>> m_local_shifts_vector;
            std::atomic<int> m_n_files_aligned = 0;
            unsigned int m_n_cpu = 1;
            std::unique_ptr<ReferencePhotoHandlerBase> m_reference_photo_handler = nullptr;
            const std::string c_reference_file_header = "reference_file";

            std::string m_alignment_method = "stars";

            std::unique_ptr<ReferencePhotoHandlerBase> reference_photo_handler_factory(const std::string& alignment_method) const;
    };
}