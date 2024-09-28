#pragma once

#include "../../headers/LocalShiftsHandler.h"

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <ostream>


/**
 * @brief Enum class for file (image) types
*/
enum class FileTypes    {
    FLAT,
    LIGHT,
    DARK,
    BIAS,
    UNKNOWN
};

/**
 * @brief Convert FileTypes enum to string
 *
 * @param type FileTypes enum
 * @return std::string string representation of the FileTypes enum
*/
std::string to_string(FileTypes type);

/**
 * @brief Convert string to FileTypes enum
 *
 * @param type string representation of the FileTypes enum
 * @return FileTypes FileTypes enum
*/
FileTypes string_to_filetype(const std::string& type);

/**
 * @brief Struct for storing alignment and ranking information for a single file
*/
struct AlignmentFileInfo {
    float shift_x            = 0;
    float shift_y            = 0;
    float rotation_center_x  = 0;
    float rotation_center_y  = 0;
    float rotation           = 0;
    float ranking            = 0;
    bool  initialized        = false;
    AstroPhotoStacker::LocalShiftsHandler local_shifts_handler;
};

std::ostream& operator<<(std::ostream& os, const AlignmentFileInfo& alignment_info);

/**
 * @brief Rearange vector ordering according to indices
 *
 * @tparam T type of the vector
 * @param vec pointer to the vector
 * @param indices pointer to the indices
*/
template<typename T>
void rearange_vector(std::vector<T> *vec, const unsigned int *indices) {
    std::vector<T> temp_vec;
    temp_vec.reserve(vec->size());
    for (unsigned int i = 0; i < vec->size(); i++) {
        temp_vec.push_back(vec->at(indices[i]));
    }
    *vec = temp_vec;
}

/**
 * @brief Class for handling list of files in all file types
*/
class FilelistHandler   {
    public:
        /**
         * @brief Construct a new Filelist Handler object
        */
        FilelistHandler();

        ~FilelistHandler() = default;

        /**
         * @brief Get the filelist with checked files
         *
         * @return FilelistHandler filelist with checked files
        */
        FilelistHandler get_filelist_with_checked_files() const;

        /**
         * @brief Add file to the list
         *
         * @param path path to the file
         * @param type type of the file
         * @param checked whether the file is checked
         * @param alignment_info alignment information for the file
        */
        void add_file(const std::string& path, FileTypes type, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo());

        /**
         * @brief Remove file from the list
         *
         * @param path path to the file
         * @param type type of the file
        */
        void remove_file(const std::string& path, FileTypes type);

        /**
         * @brief Remove file from the list
         *
         * @param file_index total index of the file - it is affected also by the ordering of the file types
        */
        void remove_file(int file_index);

        /**
         * @brief Get the files
         *
         * @param type type of the files
         * @return const std::vector<std::string>& vector of the file paths
        */
        const std::vector<std::string>& get_files(FileTypes type)   const;

        /**
         * @brief Get the files checked
         *
         * @param type type of the files
         * @return const std::vector<bool>& is the file with the given index checked?
        */
        const std::vector<bool>&        get_files_checked(FileTypes type)   const;

        /**
         * @brief Get the checked files
         *
         * @param type type of the files
         * @return std::vector<std::string> vector of the file paths
         */
        std::vector<std::string> get_checked_files(FileTypes type) const;

        /**
         * @brief Get the number of checked files of a given type
         *
         * @param type type of the files
         * @return int number of checked files
        */
        int get_number_of_checked_files(FileTypes type) const;

        /**
         * @brief Get the number of all files
         *
         * @return int number of all files
        */
        int get_number_of_all_files() const;

        /**
         * @brief Is file with a given index checked?
         *
         * @param file_index index of the file
         * @return true if the file is checked
        */
        bool file_is_checked(int file_index) const;

        /**
         * @brief Set if the file checked or not
         *
         * @param file_index index of the file
         * @param checked is the file checked?
         * @param type type of the file
        */
        void set_file_checked(int file_index, bool checked, FileTypes type);

        /**
         * @brief Set if the file checked or not
         *
         * @param file_index - total index of the file
         * @param checked is the file checked?
        */
        void set_file_checked(int file_index, bool checked);

        /**
         * @brief Set checked status for all files
         *
         * @param checked should all files be checked?
        */
        void set_checked_status_for_all_files(bool checked);

        /**
         * @brief Get the alignment info for all files of a given type
         *
         * @param type - type of the files
         * @return const std::vector<AlignmentFileInfo>& vector of the alignment information for all files of a given type
        */
        const std::vector<AlignmentFileInfo>& get_alignment_info(FileTypes type)   const;

        /**
         * @brief Get the alignment info for a single file
         *
         * @param file_index total index of the file
         * @return const AlignmentFileInfo& alignment information for the file
        */
        void set_alignment_info(int file_index, const AlignmentFileInfo& alignment_info);

        /**
         * @brief Order the files according to the file types ordering - which type of frames should be first, second etc.
        */
        static const std::vector<FileTypes>  s_file_types_ordering;

        /**
         * @brief Are all checked light frames aligned?
         *
         * @return bool are all checked light frames aligned?
        */
        bool all_checked_files_are_aligned() const;

        /**
         * @brief Get the alignment info as a tabular data for GUI
         *
         * @param tabular_data pointer to the tabular data as a vector of vectors of strings (cells of table)
         * @param description pointer to the description of the tabular data (column names)
        */
        void get_alignment_info_tabular_data(std::vector<std::vector<std::string>> *tabular_data, std::vector<std::string> *description) const;

        /**
         * @brief Save alignment information to a text file
         *
         * @param output_address path to the output file
        */
        void save_alignment_to_file(const std::string &output_address);

        /**
         * @brief Load alignment information from a text file
         *
         * @param input_address path to the input file
        */
        void load_alignment_from_file(const std::string &input_address);

        /**
         * @brief Sort files by alignment ranking
         *
         * @param ascending should the files be sorted in ascending order?
        */
        void sort_by_alignment_ranking(bool ascending = true);

        /**
         * @brief Sort files by filename
         *
         * @param ascending should the files be sorted in ascending order?
        */
        void sort_by_filename(bool ascending = true);

        /**
         * @brief Remove all files of a given type
        */
        void remove_all_files_of_selected_type(FileTypes type);

        /**
         * @brief: Keep only best N files
         */
        void keep_best_n_files(unsigned int n);

        void set_local_shifts(int i_file, const std::vector<std::tuple<int,int,int,int,bool>> &shifts);

    private:
        std::map<FileTypes, std::vector<std::string>>       m_filelist;
        std::map<FileTypes, std::vector<bool>>              m_filelist_checked;
        std::vector<AlignmentFileInfo>                      m_filelist_alignment_info;  // This is only used for light frames

};