#include "../headers/FilelistHandler.h"
#include "../../headers/Common.h"
#include "../../headers/VideoReader.h"

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace AstroPhotoStacker;
using namespace std;


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
    for (FileTypes type : s_file_types_ordering)   {
        m_filelist_checked[type] = std::vector<bool>();
        m_filelist[type]         = std::vector<InputFrame>();
    }
};

FilelistHandler FilelistHandler::get_filelist_with_checked_frames() const {
    FilelistHandler filelist_with_checked_files;
    for (FileTypes type : s_file_types_ordering)   {
        const std::vector<InputFrame>       &input_frames  = m_filelist.at(type);
        const std::vector<bool>             &files_checked = m_filelist_checked.at(type);
        for (unsigned int i = 0; i < input_frames.size(); ++i)   {
            if (files_checked[i])   {
                filelist_with_checked_files.add_frame(input_frames[i], type, true, m_filelist_alignment_info[i]);
            }
        }
    }
    return filelist_with_checked_files;
};

void FilelistHandler::add_frame(const InputFrame &input_frame, FileTypes type, bool checked, const AlignmentFileInfo& alignment_info, const Metadata &metadata) {
    m_filelist[type].push_back(input_frame);
    m_filelist_checked[type].push_back(checked);
    if (type == FileTypes::LIGHT)   {
        m_filelist_alignment_info.push_back(alignment_info);
        m_filelist_metadata.push_back(metadata);
    }
};

void FilelistHandler::add_file(const std::string &file, FileTypes type, bool checked, const AlignmentFileInfo& alignment_info, const AstroPhotoStacker::Metadata &metadata)    {
    if (is_valid_video_file(file))   {
        const std::vector<InputFrame> video_frames = get_video_frames(file);
        for (const InputFrame &video_frame : video_frames)   {
            add_frame(video_frame, type, checked, alignment_info, metadata);
        }
    }
    else    {
        add_frame(InputFrame(file), type, checked, alignment_info, metadata);
    }
};

void FilelistHandler::remove_frame(const InputFrame &input_frame, FileTypes type)  {
    std::vector<InputFrame> &input_frames   = m_filelist[type];
    std::vector<bool>        &files_checked = m_filelist_checked[type];

    const unsigned int n_files = input_frames.size();

    for (int i = n_files-1; i >= 0; i--)   {
        if (input_frames[i] == input_frame)   {
            input_frames.erase(input_frames.begin() + i);
            files_checked.erase(files_checked.begin() + i);
            if (type == FileTypes::LIGHT)   {
                m_filelist_alignment_info.erase(m_filelist_alignment_info.begin() + i);
                m_filelist_metadata.erase(m_filelist_metadata.begin() + i);
            }
        }
    }
};

void FilelistHandler::remove_frame(int file_index)   {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_files = m_filelist.at(type).size();
        if (file_index < files_previous + n_files)   {
            m_filelist[type].erase(m_filelist[type].begin() + file_index - files_previous);
            m_filelist_checked[type].erase(m_filelist_checked[type].begin() + file_index - files_previous);
            if (type == FileTypes::LIGHT)   {
                m_filelist_alignment_info.erase(m_filelist_alignment_info.begin() + file_index - files_previous);
                m_filelist_metadata.erase(m_filelist_metadata.begin() + file_index - files_previous);
            }
            return;
        }
        files_previous += n_files;
    }
};

const std::vector<InputFrame>& FilelistHandler::get_frames(FileTypes type)    const  {
    return m_filelist.at(type);
};

const std::vector<bool>& FilelistHandler::get_frames_checked(FileTypes type)   const   {
    return m_filelist_checked.at(type);
};

std::vector<InputFrame> FilelistHandler::get_checked_frames(FileTypes type) const    {
    const std::vector<InputFrame>  &input_frames  = m_filelist.at(type);
    const std::vector<bool>        &files_checked = m_filelist_checked.at(type);
    std::vector<InputFrame> checked_frames;
    for (unsigned int i = 0; i < input_frames.size(); ++i)   {
        if (files_checked[i])   {
            checked_frames.push_back(input_frames[i]);
        }
    }
    return checked_frames;
};

int FilelistHandler::get_number_of_checked_frames(FileTypes type) const  {
    const std::vector<bool> &frames_checked = m_filelist_checked.at(type);
    int n_checked_frames = 0;
    for (bool checked : frames_checked)   {
        if (checked)    {
            n_checked_frames++;
        }
    }
    return n_checked_frames;
};

int FilelistHandler::get_number_of_all_frames() const    {
    int n_files = 0;
    for (FileTypes type : s_file_types_ordering)   {
        n_files += m_filelist.at(type).size();
    }
    return n_files;
};

bool FilelistHandler::frame_is_checked(int frame_index) const {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_files = m_filelist.at(type).size();
        if (frame_index < files_previous + n_files)   {
            return m_filelist_checked.at(type)[frame_index - files_previous];
        }
        files_previous += n_files;
    }
    return false;
};

void FilelistHandler::set_frame_checked(int frame_index, bool checked, FileTypes type) {
    m_filelist_checked[type][frame_index] = checked;
};

void FilelistHandler::set_frame_checked(int frame_index, bool checked) {
    int files_previous = 0;
    for (FileTypes type : s_file_types_ordering)   {
        const int n_frames = m_filelist[type].size();
        if (frame_index < files_previous + n_frames)   {
            m_filelist_checked[type][frame_index - files_previous] = checked;
            return;
        }
        files_previous += n_frames;
    }
};

void FilelistHandler::set_checked_status_for_all_frames(bool checked)   {
    for (FileTypes type : s_file_types_ordering)   {
        std::vector<bool> &files_checked = m_filelist_checked[type];
        for (unsigned int i = 0; i < files_checked.size(); ++i)   {
            files_checked[i] = checked;
        }
    }
};

const std::vector<AlignmentFileInfo>& FilelistHandler::get_alignment_info()   const    {
    return m_filelist_alignment_info;
};

const std::vector<Metadata>& FilelistHandler::get_metadata()   const   {
    return m_filelist_metadata;
};

void FilelistHandler::set_alignment_info(int frame_index, const AlignmentFileInfo& alignment_info)    {
    m_filelist_alignment_info[frame_index] = alignment_info;
};

bool FilelistHandler::all_checked_frames_are_aligned() const {
    const std::vector<bool> &light_frames_checked = m_filelist_checked.at(FileTypes::LIGHT);
    if (light_frames_checked.size() == 0)    {
        return false;
    }
    const std::vector<AlignmentFileInfo> &alignment_info = m_filelist_alignment_info;
    for (unsigned int i = 0; i < light_frames_checked.size(); ++i)   {
        if (light_frames_checked[i] && !alignment_info[i].initialized)   {
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
    const std::vector<InputFrame> &light_frames = m_filelist.at(FileTypes::LIGHT);
    const std::vector<AlignmentFileInfo> &alignment_info = m_filelist_alignment_info;

    for (unsigned int i = 0; i < light_frames.size(); ++i)   {
        const AlignmentFileInfo &info = alignment_info[i];
        const std::string file_description = light_frames[i].is_video_frame() ?
                light_frames[i].get_file_address() + " #frame: " + std::to_string(light_frames[i].get_frame_number()) :
                light_frames[i].get_file_address();
        tabular_data->push_back({file_description, std::to_string(info.shift_x), std::to_string(info.shift_y), std::to_string(info.rotation_center_x), std::to_string(info.rotation_center_y), std::to_string(info.rotation), std::to_string(info.ranking)});
    }
};

void FilelistHandler::save_alignment_to_file(const std::string &output_address)  {
    const std::vector<AlignmentFileInfo> &alignment_info = get_alignment_info();
    const std::vector<InputFrame> &light_frames = get_frames(FileTypes::LIGHT);
    std::ofstream output_file(output_address);
    for (unsigned int i_file = 0; i_file < alignment_info.size(); ++i_file)   {
        const AlignmentFileInfo &info = alignment_info[i_file];
        if (!info.initialized)  {
            continue;
        }
        output_file << light_frames[i_file].get_file_address() << " | "
                    << light_frames[i_file].get_frame_number() << " | "
                    << info.shift_x << " | "
                    << info.shift_y << " | "
                    << info.rotation_center_x << " | "
                    << info.rotation_center_y << " | "
                    << info.rotation << " | "
                    << info.ranking << std::endl;
    }
};

void FilelistHandler::load_alignment_from_file(const std::string &input_address)    {
    std::ifstream input_file(input_address);
    std::string line;
    while (std::getline(input_file, line))   {
        vector<string> elements = split_string(line, " | ");
        if (elements.size() != 7)   {
            continue;
        }
        const std::string file_address = elements[0];
        const int frame_number         = std::stoi(elements[1]);
        const InputFrame input_frame(file_address, frame_number);

        AlignmentFileInfo alignment_info;
        alignment_info.shift_x           = std::stof(elements[2]);
        alignment_info.shift_y           = std::stof(elements[3]);
        alignment_info.rotation_center_x = std::stof(elements[4]);
        alignment_info.rotation_center_y = std::stof(elements[5]);
        alignment_info.rotation          = std::stof(elements[6]);
        alignment_info.ranking           = std::stof(elements[7]);
        alignment_info.initialized       = true;


        const std::vector<InputFrame> &light_files = get_frames(FileTypes::LIGHT);
        for (unsigned int i_file = 0; i_file < light_files.size(); ++i_file)   {
            if (light_files[i_file] == input_frame)   {
                set_alignment_info(i_file, alignment_info);
                break;
            }
        }

    }
};

void FilelistHandler::sort_by_alignment_ranking(bool ascending)   {
    vector<pair<float, unsigned int>> ranking_index;
    for (unsigned int i = 0; i < m_filelist_alignment_info.size(); ++i)   {
        ranking_index.push_back({m_filelist_alignment_info[i].ranking, i});
    }

    if (ascending)  {
        sort(ranking_index.begin(), ranking_index.end());
    }
    else    {
        sort(ranking_index.begin(), ranking_index.end(), greater<pair<float, int>>());
    }

    vector<unsigned int> indices(ranking_index.size());
    for (unsigned int i = 0; i < ranking_index.size(); ++i)   {
        indices[i] = ranking_index[i].second;
    }

    rearange_vector(&m_filelist.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_checked.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_alignment_info, indices.data());
    rearange_vector(&m_filelist_metadata, indices.data());
};

void FilelistHandler::sort_by_filename(bool ascending)    {
    vector<pair<InputFrame, unsigned int>> input_frames_and_indices;
    for (unsigned int i = 0; i < m_filelist_alignment_info.size(); ++i)   {
        input_frames_and_indices.push_back({m_filelist.at(FileTypes::LIGHT)[i], i});
    }

    if (ascending)  {
        sort(input_frames_and_indices.begin(), input_frames_and_indices.end());
    }
    else    {
        sort(input_frames_and_indices.begin(), input_frames_and_indices.end(), greater<pair<InputFrame, int>>());
    }

    vector<unsigned int> indices(input_frames_and_indices.size());
    for (unsigned int i = 0; i < input_frames_and_indices.size(); ++i)   {
        indices[i] = input_frames_and_indices[i].second;
    }

    rearange_vector(&m_filelist.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_checked.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_alignment_info, indices.data());
    rearange_vector(&m_filelist_metadata, indices.data());
};

void FilelistHandler::remove_all_frames_of_selected_type(FileTypes type)  {
    m_filelist[type].clear();
    m_filelist_checked[type].clear();
    if (type == FileTypes::LIGHT)   {
        m_filelist_alignment_info.clear();
    }
};

void FilelistHandler::keep_best_n_frames(unsigned int n)   {
    if (m_filelist[FileTypes::LIGHT].size() <= n) {
        return;
    }

    vector<pair<float, unsigned int>> ranking_index;
    for (unsigned int i = 0; i < m_filelist_alignment_info.size(); ++i)   {
        ranking_index.push_back({m_filelist_alignment_info[i].ranking, i});
    }

    sort(ranking_index.begin(), ranking_index.end(), [](const pair<float, unsigned int> &a, const pair<float, unsigned int> &b) {
        return a.first < b.first;
    });

    vector<unsigned int> indices(ranking_index.size());
    for (unsigned int i = 0; i < ranking_index.size(); ++i)   {
        indices[i] = ranking_index[i].second;
    }

    if (n > indices.size()) {
        n = indices.size();
    }

    rearange_vector(&m_filelist.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_checked.at(FileTypes::LIGHT), indices.data());
    rearange_vector(&m_filelist_alignment_info, indices.data());
    rearange_vector(&m_filelist_metadata, indices.data());

    m_filelist.at(FileTypes::LIGHT).resize(n);
    m_filelist_checked.at(FileTypes::LIGHT).resize(n);
    m_filelist_alignment_info.resize(n);
    m_filelist_metadata.resize(n);
};

void FilelistHandler::set_local_shifts(int i_file, const std::vector<LocalShift> &shifts)   {
    m_filelist_alignment_info[i_file].local_shifts_handler = LocalShiftsHandler(shifts);
};