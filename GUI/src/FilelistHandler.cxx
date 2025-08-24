#include "../headers/FilelistHandler.h"
#include "../../headers/Common.h"
#include "../../headers/VideoReader.h"
#include "../../headers/TaskScheduler.hxx"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace AstroPhotoStacker;
using namespace std;

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

const std::vector<FrameType>  FilelistHandler::s_file_types_ordering = {FrameType::LIGHT, FrameType::FLAT, FrameType::DARK, FrameType::BIAS, FrameType::UNKNOWN};

FilelistHandler FilelistHandler::get_filelist_with_checked_frames() const {
    FilelistHandler filelist_with_checked_files;
    for (const auto &group : m_frames_list)   {
        filelist_with_checked_files.add_empty_group(group.first);
        for (const auto &type : group.second)   {
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames)   {
                if (frame.second.is_checked)   {
                    filelist_with_checked_files.add_frame(frame.second.input_frame, type.first, group.first, true, frame.second.alignment_info, frame.second.metadata);
                }
            }
        }
    }
    return filelist_with_checked_files;
};

void FilelistHandler::add_frame(const InputFrame &input_frame, FrameType type, int group, bool checked, const AlignmentFileInfo& alignment_info, const Metadata &metadata) {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        add_empty_group(group);
    }

    std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = m_frames_list[group][type];
    FrameInfo frame_info;
    frame_info.input_frame = input_frame;
    frame_info.type = type;
    frame_info.group_number = group;
    frame_info.alignment_info = alignment_info;
    frame_info.metadata = metadata;
    frame_info.is_checked = checked;

    frames[input_frame] = frame_info;
};

void FilelistHandler::add_file(const std::string &file, FrameType type, int group, bool checked, const AlignmentFileInfo& alignment_info, const AstroPhotoStacker::Metadata &metadata)    {
    if (is_valid_video_file(file))   {
        const std::vector<InputFrame> video_frames = get_video_frames(file);
        for (const InputFrame &video_frame : video_frames)   {
            add_frame(video_frame, type, group, checked, alignment_info, metadata);
        }
    }
    else    {
        add_frame(InputFrame(file), type, group, checked, alignment_info, metadata);
    }
};

void FilelistHandler::remove_frame(const InputFrame &input_frame, int group, FrameType type)  {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        return;
    }

    std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = m_frames_list[group][type];
    if (frames.find(input_frame) != frames.end())   {
        frames.erase(input_frame);
    }
};

void FilelistHandler::keep_only_frames_satisfying_condition(const std::function<bool(FrameType, int, const FrameInfo&)> &keep_condition)   {
    std::map<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>> new_map;
    for (auto &group : m_frames_list)   {
        new_map[group.first] = std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>();
        for (auto &type : group.second)   {
            new_map[group.first][type.first] = std::map<AstroPhotoStacker::InputFrame,FrameInfo>();
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = type.second;
            for (const auto &frame : frames)   {
                if (keep_condition(type.first, group.first, frame.second))   {
                    new_map[group.first][type.first][frame.first] = frame.second;
                }
            }
        }
    }
};


const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &FilelistHandler::get_frames(FrameType type, int group)    const  {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        throw std::runtime_error("FilelistHandler::get_frames: group not found");
    }
    if (m_frames_list.at(group).find(type) == m_frames_list.at(group).end())   {
        throw std::runtime_error("FilelistHandler::get_frames: type not found");
    }
    return m_frames_list.at(group).at(type);
};

vector<AstroPhotoStacker::InputFrame> FilelistHandler::get_selected_frames() const {
    std::vector<AstroPhotoStacker::InputFrame> selected_frames;
    for (const auto &group : m_frames_list)   {
        for (const auto &type : group.second)   {
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames)   {
                if (frame.second.is_checked)   {
                    selected_frames.push_back(frame.first);
                }
            }
        }
    }
    return selected_frames;
};

int FilelistHandler::get_number_of_checked_frames(FrameType type) const  {
    int n_checked_frames = 0;
    for (const auto &group : m_frames_list)   {
        if (group.second.find(type) == group.second.end())   {
            continue;
        }
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames = group.second.at(type);
        for (const auto &frame : frames)   {
            if (frame.second.is_checked)   {
                n_checked_frames++;
            }
        }
    }
    return n_checked_frames;
};


int FilelistHandler::get_number_of_all_frames(FrameType type) const  {
    int n_files = 0;
    for (const auto &group : m_frames_list)   {
        if (group.second.find(type) == group.second.end())   {
            continue;
        }
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames = group.second.at(type);
        n_files += frames.size();
    }
    return n_files;
};

int FilelistHandler::get_number_of_all_frames() const    {
    int n_files = 0;
    for (const auto &group : m_frames_list)   {
        for (const auto &type : group.second)   {
            const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames = group.second.at(type.first);
            n_files += frames.size();
        }
    }
    return n_files;
};

bool FilelistHandler::frame_is_checked(int group, const AstroPhotoStacker::InputFrame &input_frame, FrameType type) const {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        return false;
    }
    if (m_frames_list.at(group).find(type) == m_frames_list.at(group).end())   {
        return false;
    }
    const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = m_frames_list.at(group).at(type);
    if (frames.find(input_frame) == frames.end())   {
        return false;
    }
    return frames.at(input_frame).is_checked;
};

void FilelistHandler::set_frame_checked(int group, const AstroPhotoStacker::InputFrame &input_frame, FrameType type, bool checked) {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        return;
    }
    if (m_frames_list[group].find(type) == m_frames_list[group].end())   {
        return;
    }
    std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = m_frames_list[group][type];
    if (frames.find(input_frame) == frames.end())   {
        return;
    }
    frames[input_frame].is_checked = checked;
};

void FilelistHandler::set_checked_status_for_all_frames(bool checked)   {
    for (auto &group : m_frames_list)   {
        for (auto &type : group.second)   {
            std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (auto &frame : frames)   {
                frame.second.is_checked = checked;
            }
        }
    }
};

void FilelistHandler::set_alignment_info(const InputFrame &input_frame, const AlignmentFileInfo& alignment_info)    {
    // the frame might be in multiple groups
    for (auto &group : m_frames_list)   {
        std::map<AstroPhotoStacker::InputFrame, FrameInfo> &light_frames = group.second[FrameType::LIGHT];
        if (light_frames.find(input_frame) == light_frames.end())   {
            continue;
        }
        light_frames[input_frame].alignment_info = alignment_info;
    }
};

bool FilelistHandler::all_checked_frames_are_aligned() const {
    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);
        for (const auto &frame : light_frames)   {
            if (frame.second.is_checked && !frame.second.alignment_info.initialized)   {
                return false;
            }
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

    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);

        for (const auto &frame : light_frames)   {
            const AlignmentFileInfo &info = frame.second.alignment_info;
            const std::string file_description = frame.second.input_frame.is_video_frame() ?
                    frame.second.input_frame.get_file_address() + " #frame: " + std::to_string(frame.second.input_frame.get_frame_number()) :
                    frame.second.input_frame.get_file_address();
            tabular_data->push_back({file_description, std::to_string(info.shift_x), std::to_string(info.shift_y), std::to_string(info.rotation_center_x), std::to_string(info.rotation_center_y), std::to_string(info.rotation), std::to_string(info.ranking)});
        }
    }
};

const AlignmentFileInfo& FilelistHandler::get_alignment_info(int group, const AstroPhotoStacker::InputFrame &input_frame) const {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: group not found");
    }
    if (m_frames_list.at(group).find(FrameType::LIGHT) == m_frames_list.at(group).end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: frame type not found");
    }
    if (m_frames_list.at(group).at(FrameType::LIGHT).find(input_frame) == m_frames_list.at(group).at(FrameType::LIGHT).end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: frame not found");
    }
    return m_frames_list.at(group).at(FrameType::LIGHT).at(input_frame).alignment_info;
};

void FilelistHandler::save_alignment_to_file(const std::string &output_address)  {
    std::ofstream output_file(output_address);
    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);

        for (const auto &frame : light_frames)   {
            const AlignmentFileInfo &info = frame.second.alignment_info;
            if (!info.initialized)  {
                continue;
            }
            output_file << frame.second.input_frame.get_file_address() << " | "
                        << frame.second.input_frame.get_frame_number() << " | "
                        << info.shift_x << " | "
                        << info.shift_y << " | "
                        << info.rotation_center_x << " | "
                        << info.rotation_center_y << " | "
                        << info.rotation << " | "
                        << info.ranking << " | "
                        << info.local_shifts_handler.to_string() << std::endl;
        }
    }
};

void FilelistHandler::load_alignment_from_file(const std::string &input_address)    {
    std::ifstream input_file(input_address);
    std::string line;
    while (std::getline(input_file, line))   {
        vector<string> elements = split_string(line, " | ");
        if (elements.size() < 8)   {
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

        if (elements.size() > 8)    {
            alignment_info.local_shifts_handler = LocalShiftsHandler(elements[8]);
        }

        set_alignment_info(input_frame, alignment_info);
    }
};

void FilelistHandler::remove_all_frames_of_selected_type(FrameType type)  {
    for (auto &group : m_frames_list)   {
        if (group.second.find(type) == group.second.end())   {
            continue;
        }
        group.second[type].clear();
    }
};

void FilelistHandler::keep_best_n_frames(unsigned int n)   {
    vector<FrameInfo> light_frames;
    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames = group.second.at(FrameType::LIGHT);
        for (const auto &frame : frames)   {
            light_frames.push_back(frame.second);
        }
    }

    std::sort(light_frames.begin(), light_frames.end(), [](const FrameInfo &a, const FrameInfo &b) {
        return a.alignment_info.ranking < b.alignment_info.ranking;
    });

    light_frames.resize(n);

    remove_all_frames_of_selected_type(FrameType::LIGHT);
    for (const FrameInfo &frame : light_frames)   {
        m_frames_list[frame.group_number][FrameType::LIGHT][frame.input_frame] = frame;
    }
};

void FilelistHandler::set_local_shifts(const AstroPhotoStacker::InputFrame &input_frame, const std::vector<LocalShift> &shifts)   {
    // the frame might be in multiple groups
    for (auto &group : m_frames_list)   {
        std::map<AstroPhotoStacker::InputFrame, FrameInfo> &light_frames = group.second[FrameType::LIGHT];
        if (light_frames.find(input_frame) == light_frames.end())   {
            continue;
        }
        light_frames[input_frame].alignment_info.local_shifts_handler = LocalShiftsHandler(shifts);
    }
};

void FilelistHandler::set_dummy_alignment_for_all_frames()  {
    for (auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }
        std::map<AstroPhotoStacker::InputFrame, FrameInfo> &light_frames = group.second[FrameType::LIGHT];
        for (auto &frame : light_frames)   {
            frame.second.alignment_info.shift_x = 0;
            frame.second.alignment_info.shift_y = 0;
            frame.second.alignment_info.rotation_center_x = 0;
            frame.second.alignment_info.rotation_center_y = 0;
            frame.second.alignment_info.rotation = 0;
            frame.second.alignment_info.ranking = 0;
            frame.second.alignment_info.initialized = true;
        }
    }
};

std::vector<int> FilelistHandler::get_group_numbers() const {
    std::vector<int> group_numbers;
    for (const auto &group : m_frames_list) {
        group_numbers.push_back(group.first);
    }
    return group_numbers;
};

std::vector<FrameInfo> FilelistHandler::get_checked_frames_of_type(FrameType type) const {
    std::vector<FrameInfo> frames;
    for (const auto &group : m_frames_list)   {
        if (group.second.find(type) == group.second.end())   {
            continue;
        }
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames_map = group.second.at(type);
        for (const auto &frame : frames_map)   {
            if (frame.second.is_checked)   {
                frames.push_back(frame.second);
            }
        }
    }
    return frames;
};

void FilelistHandler::remove_all_frames_of_type_and_group(FrameType type, int group)    {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        return;
    }
    if (m_frames_list[group].find(type) == m_frames_list[group].end())   {
        return;
    }
    m_frames_list[group][type].clear();
};

void FilelistHandler::remove_group(int group_number)    {
    if (m_frames_list.find(group_number) == m_frames_list.end())   {
        return;
    }
    m_frames_list.erase(group_number);
};


void FilelistHandler::add_empty_group(int group_number) {
    if (m_frames_list.find(group_number) == m_frames_list.end())   {
        m_frames_list[group_number] = std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>();
    }
    for (FrameType type : s_file_types_ordering)   {
        if (m_frames_list[group_number].find(type) == m_frames_list[group_number].end())   {
            m_frames_list[group_number][type] = std::map<AstroPhotoStacker::InputFrame,FrameInfo>();
        }
    }
};


void FilelistHandler::save_filelist_to_file(const std::string &output_address)   {
    std::ofstream output_file(output_address);
    for (const auto &group : m_frames_list)   {
        for (const auto &type : group.second)   {
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames)   {
                const FrameInfo &frame_info = frame.second;
                output_file << frame_info.input_frame.get_file_address() << " | "
                            << frame_info.input_frame.get_frame_number() << " | "
                            << to_string(frame_info.type) << " | "
                            << frame_info.group_number << " | "
                            << frame_info.is_checked << std::endl;
            }
        }
    }
};

void FilelistHandler::load_filelist_from_file(const std::string &input_address)  {
    m_frames_list.clear();
    std::ifstream input_file(input_address);
    std::string line;
    while (std::getline(input_file, line))   {
        vector<string> elements = split_string(line, " | ");
        if (elements.size() != 5)   {
            continue;
        }
        const std::string file_address = elements[0];
        const int frame_number         = std::stoi(elements[1]);
        const FrameType type           = string_to_filetype(elements[2]);
        const int group_number         = std::stoi(elements[3]);
        const bool is_checked          = static_cast<bool>(std::stoi(elements[4]));

        if (!std::filesystem::exists(file_address))     continue;
        if (type == FrameType::UNKNOWN)                 continue;

        const InputFrame input_frame(file_address, frame_number);
        add_frame(input_frame, type, group_number, is_checked);
    }
};

void FilelistHandler::calculate_frame_statistics(unsigned int n_cpu, std::atomic<int> *counter) {
    AstroPhotoStacker::TaskScheduler task_scheduler({n_cpu});
    auto process_frame = [counter, n_cpu, &task_scheduler](AstroPhotoStacker::FrameStatistics *frame_statistics, const AstroPhotoStacker::InputFrame &input_frame) {
        if (n_cpu > 1) {
            task_scheduler.submit([frame_statistics, input_frame, counter]() {
                *frame_statistics = AstroPhotoStacker::get_frame_statistics(input_frame);
                if (counter) {
                    (*counter)++;
                }
            }, {1});
        }
        else {
            *frame_statistics = AstroPhotoStacker::get_frame_statistics(input_frame);
            if (counter) {
                (*counter)++;
            }
        }
    };

    for (auto &group : m_frames_list) {
        for (auto &type : group.second) {
            std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (auto &frame : frames) {
                FrameInfo &frame_info = frame.second;
                if (!frame_info.statistics.is_valid) {
                    process_frame(&frame_info.statistics, frame.first);
                }
            }
        }
    }
}

int FilelistHandler::get_number_of_frames_without_statistics() const    {
    int count = 0;
    for (const auto &group : m_frames_list) {
        for (const auto &type : group.second) {
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames) {
                const FrameInfo &frame_info = frame.second;
                if (!frame_info.statistics.is_valid) {
                    count++;
                }
            }
        }
    }
    return count;
};

bool FilelistHandler::statistics_calculated_for_all_frames() const {
    for (const auto &group : m_frames_list) {
        for (const auto &type : group.second) {
            const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames) {
                const FrameInfo &frame_info = frame.second;
                if (!frame_info.statistics.is_valid) {
                    return false;
                }
            }
        }
    }
    return true;
}
