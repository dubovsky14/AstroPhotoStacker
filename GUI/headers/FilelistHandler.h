#pragma once

#include "../../headers/LocalShiftsHandler.h"
#include "../../headers/Metadata.h"
#include "../../headers/InputFrame.h"

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
        FilelistHandler get_filelist_with_checked_frames() const;

        /**
         * @brief Add file to the list
         *
         * @param input_frame - input frame data
         * @param type type of the file
         * @param checked whether the file is checked
         * @param alignment_info alignment information for the file
        */
        void add_frame(const AstroPhotoStacker::InputFrame &input_frame, FileTypes type, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo(), const AstroPhotoStacker::Metadata &metadata = AstroPhotoStacker::Metadata());

        /**
         * @brief Add file to the list
         *
         * @param path path to the file
         * @param type type of the file
         * @param checked whether the file is checked
         * @param alignment_info alignment information for the file
        */
        void add_file(const std::string &file, FileTypes type, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo(), const AstroPhotoStacker::Metadata &metadata = AstroPhotoStacker::Metadata());

        /**
         * @brief Remove file from the list
         *
         * @param path path to the file
         * @param type type of the file
        */
        void remove_frame(const AstroPhotoStacker::InputFrame &input_frame, FileTypes type);

        /**
         * @brief Remove file from the list
         *
         * @param file_index total index of the file - it is affected also by the ordering of the file types
        */
        void remove_frame(int file_index);

        /**
         * @brief Get the frames
         *
         * @param type type of the frames
         * @return const std::vector<AstroPhotoStacker::InputFrame>& vector of the file paths
        */
        const std::vector<AstroPhotoStacker::InputFrame>& get_frames(FileTypes type)   const;

        /**
         * @brief Get the checked frames
         *
         * @param type type of the frames
         * @return const std::vector<bool>& is the file with the given index checked?
        */
        const std::vector<bool>&        get_frames_checked(FileTypes type)   const;

        /**
         * @brief Get the checked frames
         *
         * @param type type of the frames
         * @return std::vector<AstroPhotoStacker::InputFrame> vector of the file paths
         */
        std::vector<AstroPhotoStacker::InputFrame> get_checked_frames(FileTypes type) const;

        /**
         * @brief Get the number of checked frames of a given type
         *
         * @param type type of the frames
         * @return int number of checked frames
        */
        int get_number_of_checked_frames(FileTypes type) const;

        /**
         * @brief Get the number of all frames
         *
         * @return int number of all frames
        */
        int get_number_of_all_frames() const;

        /**
         * @brief Is file with a given index checked?
         *
         * @param frame_index index of the file
         * @return true if the file is checked
        */
        bool frame_is_checked(int frame_index) const;

        /**
         * @brief Set if the file checked or not
         *
         * @param frame_index index of the file
         * @param checked is the file checked?
         * @param type type of the file
        */
        void set_frame_checked(int frame_index, bool checked, FileTypes type);

        /**
         * @brief Set if the file checked or not
         *
         * @param frame_index - total index of the file
         * @param checked is the file checked?
        */
        void set_frame_checked(int frame_index, bool checked);

        /**
         * @brief Set checked status for all frames
         *
         * @param checked should all frames be checked?
        */
        void set_checked_status_for_all_frames(bool checked);

        /**
         * @brief Get the alignment info for all frames of a given type
         *
         * @param type - type of the frames
         * @return const std::vector<AlignmentFileInfo>& vector of the alignment information for all frames of a given type
        */
        const std::vector<AlignmentFileInfo>& get_alignment_info()   const;

        /**
         * @brief Get the metadata for all frames of a given type
         *
         * @param type - type of the frames
         * @return const std::vector<Metadata>& vector of the metadata for all frames of a given type
        */
        const std::vector<AstroPhotoStacker::Metadata>& get_metadata()   const;

        /**
         * @brief Get the alignment info for a single frame
         *
         * @param file_index total index of the frame
         * @return const AlignmentFileInfo& alignment information for the frame
        */
        void set_alignment_info(int frame_index, const AlignmentFileInfo& alignment_info);

        /**
         * @brief Order the frames according to the file types ordering - which type of frames should be first, second etc.
        */
        static const std::vector<FileTypes>  s_file_types_ordering;

        /**
         * @brief Are all checked light frames aligned?
         *
         * @return bool are all checked light frames aligned?
        */
        bool all_checked_frames_are_aligned() const;

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
        void remove_all_frames_of_selected_type(FileTypes type);

        /**
         * @brief: Keep only best N files
         */
        void keep_best_n_frames(unsigned int n);

        void set_local_shifts(int i_file, const std::vector<AstroPhotoStacker::LocalShift> &shifts);

        AstroPhotoStacker::InputFrame get_input_frame_by_gui_string(const std::string &file_description) const;

    private:
        std::map<FileTypes, std::vector<AstroPhotoStacker::InputFrame>>     m_filelist;
        std::map<FileTypes, std::vector<bool>>                              m_filelist_checked;

        // #TODO: clean up this mess
        std::vector<AlignmentFileInfo>                      m_filelist_alignment_info;  // This is only used for light frames
        std::vector<AstroPhotoStacker::Metadata>            m_filelist_metadata;        // This is only used for light frames

};