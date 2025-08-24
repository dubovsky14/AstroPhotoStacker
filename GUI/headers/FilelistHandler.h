#pragma once

#include "../../headers/FrameStatistics.h"
#include "../../headers/LocalShiftsHandler.h"
#include "../../headers/Metadata.h"
#include "../../headers/InputFrame.h"
#include "../headers/FrameType.h"

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <ostream>
#include <functional>
#include <set>
#include <atomic>


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

struct FrameInfo {
    AlignmentFileInfo                       alignment_info;
    AstroPhotoStacker::Metadata             metadata;
    AstroPhotoStacker::InputFrame           input_frame;
    AstroPhotoStacker::FrameStatistics      statistics;
    FrameType                               type;
    int                                     group_number;
    bool                                    is_checked = true;
};

std::ostream& operator<<(std::ostream& os, const AlignmentFileInfo& alignment_info);

/**
 * @brief Rearange vector ordering according to indices
 *
 * @tparam T type of the vector
 * @param vec pointer to the vector
 * @param indices pointer to the indices
*/
template<typename T, typename IndexType>
void rearange_vector(std::vector<T> *vec, const IndexType *indices) {
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
        FilelistHandler() = default;

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
         * @param group number of the frame the file belongs to
         * @param checked whether the file is checked
         * @param alignment_info alignment information for the file
        */
        void add_frame(const AstroPhotoStacker::InputFrame &input_frame, FrameType type, int group, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo(), const AstroPhotoStacker::Metadata &metadata = AstroPhotoStacker::Metadata());

        /**
         * @brief Add file to the list
         *
         * @param path path to the file
         * @param type type of the file
         * @param group number of the group the file belongs to
         * @param checked whether the file is checked
         * @param alignment_info alignment information for the file
        */
        void add_file(const std::string &file, FrameType type, int group, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo(), const AstroPhotoStacker::Metadata &metadata = AstroPhotoStacker::Metadata());

        /**
         * @brief Remove file from the list
         *
         * @param path path to the file
         * @param type type of the file
        */
        void remove_frame(const AstroPhotoStacker::InputFrame &input_frame, int group, FrameType type);

        /**
         * @brief Keep only frames satisfying the condition
         *
         * @param keep_condition condition for keeping the frame
         */
        void keep_only_frames_satisfying_condition(const std::function<bool(FrameType, int, const FrameInfo&)> &keep_condition);

        /**
         * @brief Get the frames
         *
         * @param type type of the frames
         * @param group number of the group the frames belong to
         * @return const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &vector of FrameInfo objects
        */
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &get_frames(FrameType type, int group)   const;

        std::vector<AstroPhotoStacker::InputFrame> get_selected_frames()  const;

        /**
         * @brief Get the number of checked frames of a given type
         *
         * @param type type of the frames
         * @return int number of checked frames
        */
        int get_number_of_checked_frames(FrameType type) const;

        /**
         * @brief Get the number of all frames of a given type
         *
         * @param type type of the frames
         * @return int number of all frames
        */
        int get_number_of_all_frames(FrameType type) const;

        /**
         * @brief Get the number of all frames
         *
         * @return int number of all frames
        */
        int get_number_of_all_frames() const;

        /**
         * @brief Is file with a given index checked?
         *
         * @return true if the file is checked
        */
        bool frame_is_checked(int group, const AstroPhotoStacker::InputFrame &input_frame, FrameType type) const;

        /**
         * @brief Set if the file checked or not
         *
         * @param frame_index index of the file
         * @param checked is the file checked?
         * @param type type of the file
        */
        void set_frame_checked(int group, const AstroPhotoStacker::InputFrame &input_frame, FrameType type, bool checked);

        /**
         * @brief Set checked status for all frames
         *
         * @param checked should all frames be checked?
        */
        void set_checked_status_for_all_frames(bool checked);

        /**
         * @brief Get the alignment info for a single frame
         *
         * @param file_index total index of the frame
         * @return const AlignmentFileInfo& alignment information for the frame
        */
        void set_alignment_info(const AstroPhotoStacker::InputFrame &input_frame, const AlignmentFileInfo& alignment_info);

        /**
         * @brief Order the frames according to the file types ordering - which type of frames should be first, second etc.
        */
        static const std::vector<FrameType>  s_file_types_ordering;

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
         * @brief Get the alignment info for a single frame
         *
         * @param group number of the group the frame belongs to
         * @param input_frame input frame data
         * @return const AlignmentFileInfo& alignment information for the frame, if frame does not exist, return empty AlignmentFileInfo
        */
        const AlignmentFileInfo &get_alignment_info(int group, const AstroPhotoStacker::InputFrame &input_frame) const;

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
         * @brief Remove all files of a given type
        */
        void remove_all_frames_of_selected_type(FrameType type);

        /**
         * @brief: Keep only best N files
         */
        void keep_best_n_frames(unsigned int n);

        void set_local_shifts(const AstroPhotoStacker::InputFrame &input_frame, const std::vector<AstroPhotoStacker::LocalShift> &shifts);

        void set_dummy_alignment_for_all_frames();

        std::vector<int> get_group_numbers() const;

        std::vector<FrameInfo> get_checked_frames_of_type(FrameType type) const;

        void remove_all_frames_of_type_and_group(FrameType type, int group);

        void remove_group(int group_number);

        void save_filelist_to_file(const std::string &output_address);

        void load_filelist_from_file(const std::string &input_address);

        void calculate_frame_statistics(unsigned int n_cpu = 1, std::atomic<int> *counter = nullptr);

        int get_number_of_frames_without_statistics() const;

        bool statistics_calculated_for_all_frames() const;

    protected:
        const std::map<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>>     &get_frames_list() const {
            return m_frames_list;
        }


    private:
        std::map<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>>     m_frames_list;

        void add_empty_group(int group_number);

};
