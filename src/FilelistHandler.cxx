#include "../headers/FilelistHandler.h"
#include "../headers/Common.h"
#include "../headers/VideoReader.h"
#include "../headers/TaskScheduler.hxx"
#include "../headers/MetadataReader.h"
#include "../headers/AlignmentResultFactory.h"
#include "../headers/PhotoAlignmentHandler.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace AstroPhotoStacker;
using namespace std;

std::ostream& operator<<(std::ostream& os, const AstroPhotoStacker::AlignmentResultBase& alignment_result) {
    os <<   "(" << alignment_result.get_description_string() <<
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
                    filelist_with_checked_files.add_frame(frame.second.input_frame, type.first, group.first, true, *frame.second.alignment_result, frame.second.metadata);
                }
            }
        }
    }
    return filelist_with_checked_files;
};

void FilelistHandler::add_frame(const InputFrame &input_frame, FrameType type, int group, bool checked, const AstroPhotoStacker::AlignmentResultBase &alignment_result, const Metadata &metadata) {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        add_empty_group(group);
    }

    std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = m_frames_list[group][type];
    FrameInfo frame_info;
    frame_info.input_frame = input_frame;
    frame_info.type = type;
    frame_info.group_number = group;
    frame_info.alignment_result = alignment_result.clone();
    frame_info.metadata = metadata;
    frame_info.is_checked = checked;

    frames[input_frame] = std::move(frame_info);
};

void FilelistHandler::add_file(const std::string &file, FrameType type, int group, bool checked, const AstroPhotoStacker::AlignmentResultBase &alignment_result)    {
    if (is_valid_video_file(file))   {
        const std::vector<InputFrame> video_frames = get_video_frames(file);
        for (const InputFrame &video_frame : video_frames)   {
        const Metadata metadata = read_metadata(video_frame);
            add_frame(video_frame, type, group, checked, alignment_result, metadata);
        }
    }
    else    {
        const Metadata metadata = read_metadata(InputFrame(file));
        add_frame(InputFrame(file), type, group, checked, alignment_result, metadata);
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

void FilelistHandler::set_alignment_info(const InputFrame &input_frame, const AlignmentResultBase& alignment_result)    {
    // the frame might be in multiple groups
    for (auto &group : m_frames_list)   {
        std::map<AstroPhotoStacker::InputFrame, FrameInfo> &light_frames = group.second[FrameType::LIGHT];
        if (light_frames.find(input_frame) == light_frames.end())   {
            continue;
        }
        light_frames[input_frame].alignment_result = alignment_result.clone();
    }
};

bool FilelistHandler::all_checked_frames_are_aligned() const {
    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);
        for (const auto &frame : light_frames)   {
            if (frame.second.is_checked && !frame.second.alignment_result->is_valid())  {
                return false;
            }
        }
    }
    return true;
};

void FilelistHandler::get_alignment_info_tabular_data(std::vector<std::vector<std::string>> *tabular_data, std::vector<std::string> *description) const  {
    description->clear();
    description->push_back("File");
    description->push_back("alignment_description");

    tabular_data->clear();

    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);

        for (const auto &frame : light_frames)   {
            const AlignmentResultBase &alignment = *frame.second.alignment_result;
            const std::string file_description = frame.second.input_frame.is_video_frame() ?
                    frame.second.input_frame.get_file_address() + " #frame: " + std::to_string(frame.second.input_frame.get_frame_number()) :
                    frame.second.input_frame.get_file_address();
            tabular_data->push_back({file_description, alignment.get_description_string()});
        }
    }
};

const AstroPhotoStacker::FrameStatistics &FilelistHandler::get_frame_statistics(int group, FrameType type, const AstroPhotoStacker::InputFrame &input_frame) const   {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        throw std::runtime_error("FilelistHandler::get_frame_statistics: group not found");
    }
    if (m_frames_list.at(group).find(type) == m_frames_list.at(group).end())   {
        throw std::runtime_error("FilelistHandler::get_frame_statistics: frame type not found");
    }
    if (m_frames_list.at(group).at(type).find(input_frame) == m_frames_list.at(group).at(type).end())   {
        throw std::runtime_error("FilelistHandler::get_frame_statistics: frame not found");
    }
    return m_frames_list.at(group).at(type).at(input_frame).statistics;
};

const AstroPhotoStacker::AlignmentResultBase&  FilelistHandler::get_alignment_info(int group, const AstroPhotoStacker::InputFrame &input_frame) const {
    if (m_frames_list.find(group) == m_frames_list.end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: group not found");
    }
    if (m_frames_list.at(group).find(FrameType::LIGHT) == m_frames_list.at(group).end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: frame type not found");
    }
    if (m_frames_list.at(group).at(FrameType::LIGHT).find(input_frame) == m_frames_list.at(group).at(FrameType::LIGHT).end())   {
        throw std::runtime_error("FilelistHandler::get_alignment_info: frame not found");
    }
    return *m_frames_list.at(group).at(FrameType::LIGHT).at(input_frame).alignment_result;
};

void FilelistHandler::save_alignment_to_file(const std::string &output_address)  {
    std::ofstream output_file(output_address);
    for (const auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }

        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &light_frames = group.second.at(FrameType::LIGHT);

        for (const auto &frame : light_frames)   {
            const AlignmentResultBase &alignment = *frame.second.alignment_result;
            if (!alignment.is_valid())  {
                continue;
            }
            output_file << frame.second.input_frame.get_file_address() << c_separator_in_file
                        << frame.second.input_frame.get_frame_number() << c_separator_in_file
                        << alignment.get_description_string() << std::endl;
        }
    }
};

void FilelistHandler::load_alignment_from_file(const std::string &input_address, std::atomic<int> *counter)    {
    std::ifstream input_file(input_address);
    std::string line;

    int internal_counter = 0; // to avoid using atomic operations too often
    const AstroPhotoStacker::AlignmentResultFactory &alignment_result_factory = AstroPhotoStacker::AlignmentResultFactory::get_instance();
    while (std::getline(input_file, line))   {
        internal_counter++;
        if (counter && (internal_counter % 100 == 0)) {
            (*counter) = internal_counter;
        }
        strip_string(&line);
        if (starts_with(line, "#") || starts_with(line, AstroPhotoStacker::PhotoAlignmentHandler::c_reference_file_header))   {
            continue;
        }
        vector<string> elements = split_string(line, c_separator_in_file);
        if (elements.size() < 3)   {
            continue;
        }
        const std::string file_address = elements[0];
        const int frame_number         = std::stoi(elements[1]);
        const InputFrame input_frame(file_address, frame_number);

        const int end_of_frame_info_position = find_nth_occurrence(line, c_separator_in_file, 2);
        if (end_of_frame_info_position == -1) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        const std::string frame_info = line.substr(0, end_of_frame_info_position);
        const vector<string> frame_info_vector = split_and_strip_string(frame_info, c_separator_in_file);

        if (!string_is_float(frame_info_vector[1])) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        unique_ptr<AlignmentResultBase> alignment_result = alignment_result_factory.create_alignment_result_from_description_string(line.substr(end_of_frame_info_position + c_separator_in_file.length()));
        set_alignment_info(input_frame, *alignment_result);
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
        if (a.alignment_result->is_valid() != b.alignment_result->is_valid())   {
            return a.alignment_result->is_valid();
        }
        return a.alignment_result->get_ranking_score() < b.alignment_result->get_ranking_score();
    });

    light_frames.resize(n);

    remove_all_frames_of_selected_type(FrameType::LIGHT);
    for (const FrameInfo &frame : light_frames)   {
        m_frames_list[frame.group_number][FrameType::LIGHT][frame.input_frame] = frame;
    }
};

void FilelistHandler::set_dummy_alignment_for_all_frames()  {
    for (auto &group : m_frames_list)   {
        if (group.second.find(FrameType::LIGHT) == group.second.end())   {
            continue;
        }
        std::map<AstroPhotoStacker::InputFrame, FrameInfo> &light_frames = group.second[FrameType::LIGHT];
        for (auto &frame : light_frames)   {
            frame.second.alignment_result->set_is_valid(true);
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
        const Metadata metadata        = AstroPhotoStacker::read_metadata(input_frame);
        add_frame(input_frame, type, group_number, is_checked, AlignmentResultDummy(), metadata);
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

void FilelistHandler::check_unaligned_frames() {
    for (auto &group : m_frames_list)   {
        for (auto &type : group.second)   {
            std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames = group.second.at(type.first);
            for (auto &frame : frames)   {
                if (type.first != FrameType::LIGHT) {
                    frame.second.is_checked = false;
                }
                else  {
                    frame.second.is_checked = !frame.second.alignment_result->is_valid();
                }
            }
        }
    }
};