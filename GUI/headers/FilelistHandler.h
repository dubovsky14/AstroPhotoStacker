#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <ostream>

enum class FileTypes    {
    FLAT,
    LIGHT,
    DARK,
    BIAS,
    UNKNOWN
};

std::string to_string(FileTypes type);

FileTypes string_to_filetype(const std::string& type);

struct AlignmentFileInfo {
    float shift_x            = 0;
    float shift_y            = 0;
    float rotation_center_x  = 0;
    float rotation_center_y  = 0;
    float rotation           = 0;
    float ranking            = 0;
    bool  initialized        = false;
};

std::ostream& operator<<(std::ostream& os, const AlignmentFileInfo& alignment_info);

template<typename T>
void rearange_vector(std::vector<T> *vec, const unsigned int *indices) {
    std::vector<T> temp_vec;
    temp_vec.reserve(vec->size());
    for (unsigned int i = 0; i < vec->size(); i++) {
        temp_vec.push_back(vec->at(indices[i]));
    }
    *vec = temp_vec;
}

class FilelistHandler   {
    public:
        FilelistHandler();
        ~FilelistHandler() = default;

        FilelistHandler get_filelist_with_checked_files() const;

        void add_file(const std::string& path, FileTypes type, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo());

        void remove_file(const std::string& path, FileTypes type);

        void remove_file(int file_index);

        const std::vector<std::string>& get_files(FileTypes type)   const;

        const std::vector<bool>&        get_files_checked(FileTypes type)   const;

        int get_number_of_checked_files(FileTypes type) const;

        int get_number_of_all_files() const;

        bool file_is_checked(int file_index) const;

        void set_file_checked(int file_index, bool checked, FileTypes type);

        void set_file_checked(int file_index, bool checked);

        void set_checked_status_for_all_files(bool checked);

        const std::vector<AlignmentFileInfo>& get_alignment_info(FileTypes type)   const;

        void set_alignment_info(int file_index, const AlignmentFileInfo& alignment_info);

        static const std::vector<FileTypes>  s_file_types_ordering;

        bool all_checked_files_are_aligned() const;

        void get_alignment_info_tabular_data(std::vector<std::vector<std::string>> *tabular_data, std::vector<std::string> *description) const;

        void save_alignment_to_file(const std::string &output_address);

        void load_alignment_from_file(const std::string &input_address);

        void sort_by_alignment_ranking(bool ascending = true);

        void sort_by_filename(bool ascending = true);

    private:
        std::map<FileTypes, std::vector<std::string>>       m_filelist;
        std::map<FileTypes, std::vector<bool>>              m_filelist_checked;
        std::vector<AlignmentFileInfo>                      m_filelist_alignment_info;  // This is only used for light frames

};