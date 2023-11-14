#include "../headers/FilelistHandler.h"

#include <algorithm>

std::string to_string(FileTypes type)   {
    switch (type)   {
        case FileTypes::FLAT:
            return "FLAT";
        case FileTypes::LIGHT:
            return "LIGHT";
        case FileTypes::DARK:
            return "DARK";
        case FileTypes::BIAS:
            return "BIAS";
        case FileTypes::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
};

FileTypes string_to_filetype(const std::string& type)   {
    if (type == "FLAT")     {
        return FileTypes::FLAT;
    }
    else if (type == "LIGHT")   {
        return FileTypes::LIGHT;
    }
    else if (type == "DARK")    {
        return FileTypes::DARK;
    }
    else if (type == "BIAS")    {
        return FileTypes::BIAS;
    }
    else    {
        return FileTypes::UNKNOWN;
    }
};

std::ostream& operator<<(std::ostream& os, const AlignmentFileInfo& alignment_info) {
    os <<   "(" << alignment_info.shift_x <<
                ", " << alignment_info.shift_y <<
                ", " << alignment_info.rotation_center_x <<
                ", " << alignment_info.rotation_center_y <<
                ", " << alignment_info.rotation <<
                ", " << alignment_info.ranking <<
                ", " << alignment_info.initialized <<
            ")";
    return os;
}

const std::vector<FileTypes>  FilelistHandler::s_file_types_ordering = {FileTypes::LIGHT, FileTypes::FLAT, FileTypes::DARK, FileTypes::BIAS, FileTypes::UNKNOWN};

FilelistHandler::FilelistHandler()   {
    m_filelist[FileTypes::FLAT]     = std::vector<std::string>();
    m_filelist[FileTypes::LIGHT]    = std::vector<std::string>();
    m_filelist[FileTypes::DARK]     = std::vector<std::string>();
    m_filelist[FileTypes::BIAS]     = std::vector<std::string>();
    m_filelist[FileTypes::UNKNOWN]  = std::vector<std::string>();
};

FilelistHandler FilelistHandler::get_filelist_with_checked_files() const {
    FilelistHandler filelist_with_checked_files;
    for (FileTypes type : s_file_types_ordering)   {
        const std::vector<std::string>          &files_names   = m_filelist.at(type);
        const std::vector<bool>                 &files_checked = m_filelist_checked.at(type);
        for (unsigned int i = 0; i < files_names.size(); ++i)   {
            if (files_checked[i])   {
                filelist_with_checked_files.add_file(files_names[i], type, true, m_filelist_alignment_info[i]);
            }
        }
    }
    return filelist_with_checked_files;
};

void FilelistHandler::add_file(const std::string& path, FileTypes type, bool checked, const AlignmentFileInfo& alignment_info)  {
    m_filelist[type].push_back(path);
    m_filelist_checked[type].push_back(checked);
    if (type == FileTypes::LIGHT)   {
        m_filelist_alignment_info.push_back(alignment_info);
    }
};

void FilelistHandler::remove_file(const std::string& path, FileTypes type)  {
    std::vector<std::string> &files_names   = m_filelist[type];
    std::vector<bool>        &files_checked = m_filelist_checked[type];

    const unsigned int n_files = files_names.size();

    for (int i = n_files-1; i >= 0; i--)   {
        if (files_names[i] == path)   {
            files_names.erase(files_names.begin() + i);
            files_checked.erase(files_checked.begin() + i);
            if (type == FileTypes::LIGHT)   {
                m_filelist_alignment_info.erase(m_filelist_alignment_info.begin() + i);
            }
        }
    }
};

void FilelistHandler::remove_file(int file_index)   {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_files = m_filelist.at(type).size();
        if (file_index < files_previous + n_files)   {
            m_filelist[type].erase(m_filelist[type].begin() + file_index - files_previous);
            m_filelist_checked[type].erase(m_filelist_checked[type].begin() + file_index - files_previous);
            if (type == FileTypes::LIGHT)   {
                m_filelist_alignment_info.erase(m_filelist_alignment_info.begin() + file_index - files_previous);
            }
            return;
        }
        files_previous += n_files;
    }
};

const std::vector<std::string>& FilelistHandler::get_files(FileTypes type)    const  {
    return m_filelist.at(type);
};

const std::vector<bool>& FilelistHandler::get_files_checked(FileTypes type)   const   {
    return m_filelist_checked.at(type);
};

bool FilelistHandler::file_is_checked(int file_index) const {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_files = m_filelist.at(type).size();
        if (file_index < files_previous + n_files)   {
            return m_filelist_checked.at(type)[file_index - files_previous];
        }
        files_previous += n_files;
    }
    return false;
};

void FilelistHandler::set_file_checked(int file_index, bool checked, FileTypes type) {
    m_filelist_checked[type][file_index] = checked;
};

void FilelistHandler::set_file_checked(int file_index, bool checked) {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_files = m_filelist[type].size();
        if (file_index < files_previous + n_files)   {
            m_filelist_checked[type][file_index - files_previous] = checked;
            return;
        }
        files_previous += n_files;
    }
};

void FilelistHandler::set_checked_status_for_all_files(bool checked)   {
    for (FileTypes type : s_file_types_ordering)   {
        std::vector<bool> &files_checked = m_filelist_checked[type];
        for (unsigned int i = 0; i < files_checked.size(); ++i)   {
            files_checked[i] = checked;
        }
    }
};

const std::vector<AlignmentFileInfo>& FilelistHandler::get_alignment_info(FileTypes type)   const    {
    return m_filelist_alignment_info;
};

void FilelistHandler::set_alignment_info(int file_index, const AlignmentFileInfo& alignment_info)    {
    m_filelist_alignment_info[file_index] = alignment_info;
};

bool FilelistHandler::all_checked_files_are_aligned() const {
    const std::vector<bool> &light_files_checked = m_filelist_checked.at(FileTypes::LIGHT);
    if (light_files_checked.size() == 0)    {
        return false;
    }
    const std::vector<AlignmentFileInfo> &alignment_info = m_filelist_alignment_info;
    for (unsigned int i = 0; i < light_files_checked.size(); ++i)   {
        if (light_files_checked[i] && !alignment_info[i].initialized)   {
            return false;
        }
    }
    return true;
};

void FilelistHandler::get_alignment_info_tabular_data(std::vector<std::vector<std::string>> *tabular_data, std::vector<std::string> *description) const  {
    description->clear();
    description->push_back("File");
    description->push_back("x-shift");
    description->push_back("y-shift");
    description->push_back("Rotation center [x]");
    description->push_back("Rotation center [y]");
    description->push_back("Rotation");
    description->push_back("Ranking");

    tabular_data->clear();
    const std::vector<std::string> &light_files = m_filelist.at(FileTypes::LIGHT);
    const std::vector<AlignmentFileInfo> &alignment_info = m_filelist_alignment_info;

    for (unsigned int i = 0; i < light_files.size(); ++i)   {
        const AlignmentFileInfo &info = alignment_info[i];
        tabular_data->push_back({light_files[i], std::to_string(info.shift_x), std::to_string(info.shift_y), std::to_string(info.rotation_center_x), std::to_string(info.rotation_center_y), std::to_string(info.rotation), std::to_string(info.ranking)});
    }
};