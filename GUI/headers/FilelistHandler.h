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

class FilelistHandler   {
    public:
        FilelistHandler();
        ~FilelistHandler() = default;

        void add_file(const std::string& path, FileTypes type, bool checked = false, const AlignmentFileInfo& alignment_info = AlignmentFileInfo());

        void remove_file(const std::string& path, FileTypes type);

        void remove_file(int file_index);

        const std::vector<std::string>& get_files(FileTypes type)   const;



        const std::vector<bool>&        get_files_checked(FileTypes type)   const;

        bool file_is_checked(int file_index) const;

        void set_file_checked(int file_index, bool checked, FileTypes type);

        void set_file_checked(int file_index, bool checked);

        void set_checked_status_for_all_files(bool checked);

        const std::vector<AlignmentFileInfo>& get_alignment_info(FileTypes type)   const;

        void set_alignment_info(int file_index, const AlignmentFileInfo& alignment_info);

        static const std::vector<FileTypes>  s_file_types_ordering;

    private:
        std::map<FileTypes, std::vector<std::string>>       m_filelist;
        std::map<FileTypes, std::vector<bool>>              m_filelist_checked;
        std::vector<AlignmentFileInfo>                      m_filelist_alignment_info;  // This is only used for light frames

};