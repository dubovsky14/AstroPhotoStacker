#include "../headers/FilelistHandlerGUIInterface.h"

#include "../../headers/Common.h"

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

FilelistHandlerGUIInterface::FilelistHandlerGUIInterface(const FilelistHandler &filelist_handler) :
    FilelistHandler(filelist_handler),
    m_sort_type(SortType::NAME),
    m_sort_ascending(true) {
    update_shown_frames();
};

void FilelistHandlerGUIInterface::sort_by_name(bool ascending)   {
    m_sort_type = SortType::NAME;
    m_sort_ascending = ascending;
    sort_frames();
};

void FilelistHandlerGUIInterface::sort_by_ranking(bool ascending)    {
    m_sort_type = SortType::RANKING;
    m_sort_ascending = ascending;
    sort_frames();
};

void FilelistHandlerGUIInterface::sort_by_group(bool ascending)  {
    m_sort_type = SortType::GROUP;
    m_sort_ascending = ascending;
    sort_frames();
};

void FilelistHandlerGUIInterface::sort_by_mean_brightness(bool ascending)  {
    m_sort_type = SortType::BRIGHTNESS_MEAN;
    m_sort_ascending = ascending;
    sort_frames();
};


std::vector<std::string> FilelistHandlerGUIInterface::get_gui_string_cells(const FrameID &frame_id)    {
    vector<string> result;
    if (m_show_group) {
        result.push_back("Group #" + std::to_string(frame_id.group_number));
    }

    const string frame_description = frame_id.input_frame.to_gui_string();
    const FrameType type = frame_id.type;
    result.push_back(to_string(type));
    result.push_back(frame_description);

    const FrameInfo &frame_info = get_frames_list().at(frame_id.group_number).at(type).at(frame_id.input_frame);

    const AstroPhotoStacker::Metadata &metadata = frame_info.metadata;
    const AlignmentFileInfo &alignment_info     = frame_info.alignment_info;
    const float alignment_score                 = alignment_info.alignment_result->get_ranking_score();
    const std::string exposure_string = metadata.exposure_time > 0.5 ?
                                        AstroPhotoStacker::round_and_convert_to_string(metadata.exposure_time) + " s" :
                                        AstroPhotoStacker::round_and_convert_to_string(metadata.exposure_time * 1000) + " ms";

    if (show_metadata()) {
        result.push_back("f/" + AstroPhotoStacker::round_and_convert_to_string(metadata.aperture));
        result.push_back(exposure_string);
        result.push_back(to_string(metadata.iso) + " ISO");
    }
    const string score_string = (type == FrameType::LIGHT) ? "score: " + AstroPhotoStacker::round_and_convert_to_string(alignment_score, 3) : "";
    result.push_back(score_string);


    std::string statistics_string = "";
    if (m_show_statistics) {
        result.push_back("mean: " + AstroPhotoStacker::round_and_convert_to_string(frame_info.statistics.brightness_avg));
        result.push_back("std: "  + AstroPhotoStacker::round_and_convert_to_string(frame_info.statistics.brightness_std));
    }

    return result;
};

void FilelistHandlerGUIInterface::keep_best_n_frames(unsigned int n) {
    FilelistHandler::keep_best_n_frames(n);
    update_shown_frames();
};

void FilelistHandlerGUIInterface::remove_frame(size_t index)    {
    if (index >= m_shown_frames.size())   {
        return;
    }
    const FrameID &frame_info = m_shown_frames[index].second;
    FilelistHandler::remove_frame(frame_info.input_frame, frame_info.group_number, frame_info.type);
    m_shown_frames.erase(m_shown_frames.begin() + index);
    update_shown_frames();
};

bool FilelistHandlerGUIInterface::frame_is_checked(size_t index)    const   {
    if (index >= m_shown_frames.size())   {
        return false;
    }
    return frame_is_checked(m_shown_frames[index].second);
};

void FilelistHandlerGUIInterface::set_frame_checked(size_t index, bool checked) {
    if (index >= m_shown_frames.size())   {
        return;
    }
    FilelistHandler::set_frame_checked(m_shown_frames[index].second.group_number, m_shown_frames[index].second.input_frame, m_shown_frames[index].second.type, checked);
};

FilelistHandlerGUIInterface FilelistHandlerGUIInterface::get_filelist_with_checked_frames() const   {
    FilelistHandler filelist_handler = FilelistHandler::get_filelist_with_checked_frames();
    FilelistHandlerGUIInterface filelist_handler_gui_interface(filelist_handler);
    return filelist_handler_gui_interface;
};

void FilelistHandlerGUIInterface::update_shown_frames()      {
    vector<vector<string>>  tabular_data;
    vector<FrameID>         frame_ids;

    const std::map<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>> &frames_list = get_frames_list();
    for (const auto &group : frames_list)   {
        for (const auto &type : group.second)   {
            const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames = group.second.at(type.first);
            for (const auto &frame : frames)   {
                FrameID frame_id(frame.second);
                tabular_data.push_back(get_gui_string_cells(frame.second));
                frame_ids.push_back(frame_id);
            }
        }
    }

    m_shown_frames.clear();
    const vector<string> formated_table = get_formated_table(tabular_data, 4*" "s);
    for (size_t i = 0; i < formated_table.size(); ++i) {
        m_shown_frames.push_back({formated_table[i], frame_ids[i]});
    }
    sort_frames();
};

void FilelistHandlerGUIInterface::sort_by_name_internal()   {
    const bool ascending = m_sort_ascending;
    const auto lambda = [ascending](const std::pair<std::string,FrameID> &a, const std::pair<std::string,FrameID> &b) {
        if (a.second.type != b.second.type) {
            return a.second.type < b.second.type;
        }

        if (ascending) {
            return a.second.input_frame < b.second.input_frame;
        } else {
            return a.second.input_frame > b.second.input_frame;
        }
    };
    std::sort(m_shown_frames.begin(), m_shown_frames.end(), lambda);
};

void FilelistHandlerGUIInterface::sort_by_ranking_internal()    {
    const bool ascending = m_sort_ascending;
    std::vector<std::tuple<size_t, float, FrameType>> index_ranking_type_vector;
    for (size_t i = 0; i < m_shown_frames.size(); ++i) {
        const FrameType type = m_shown_frames[i].second.type;
        const float ranking = type == FrameType::LIGHT ?
                              get_alignment_info(m_shown_frames[i].second.group_number, m_shown_frames[i].second.input_frame).get_ranking_score() : 0;
        index_ranking_type_vector.push_back({
            i,
            ranking,
            m_shown_frames[i].second.type
        });
    }

    std::sort(index_ranking_type_vector.begin(), index_ranking_type_vector.end(), [ascending](const std::tuple<size_t, float, FrameType> &a, const std::tuple<size_t, float, FrameType> &b) {
        if (std::get<2>(a) != std::get<2>(b)) {
            return std::get<2>(a) < std::get<2>(b);
        }

        if (ascending) {
            return std::get<1>(a) < std::get<1>(b);
        } else {
            return std::get<1>(a) > std::get<1>(b);
        }
    });

    std::vector<size_t> indices;
    for (const auto &element : index_ranking_type_vector) {
        indices.push_back(std::get<0>(element));
    }

    rearange_vector(&m_shown_frames, indices.data());
};

void FilelistHandlerGUIInterface::sort_by_mean_brightness_internal()   {
    const bool ascending = m_sort_ascending;
    std::vector<std::tuple<size_t, float, FrameType>> index_brightness_type_vector;
    for (size_t i = 0; i < m_shown_frames.size(); ++i) {
        const FrameType type = m_shown_frames[i].second.type;
        const float brightness = get_frame_statistics(m_shown_frames[i].second.group_number, type, m_shown_frames[i].second.input_frame).brightness_avg;
        index_brightness_type_vector.push_back({
            i,
            brightness,
            m_shown_frames[i].second.type
        });
    }

    std::sort(index_brightness_type_vector.begin(), index_brightness_type_vector.end(), [ascending](const std::tuple<size_t, float, FrameType> &a, const std::tuple<size_t, float, FrameType> &b) {
        if (std::get<2>(a) != std::get<2>(b)) {
            return std::get<2>(a) < std::get<2>(b);
        }

        if (ascending) {
            return std::get<1>(a) < std::get<1>(b);
        } else {
            return std::get<1>(a) > std::get<1>(b);
        }
    });

    std::vector<size_t> indices;
    for (const auto &element : index_brightness_type_vector) {
        indices.push_back(std::get<0>(element));
    }

    rearange_vector(&m_shown_frames, indices.data());
};

void FilelistHandlerGUIInterface::sort_by_group_internal()  {
    const bool ascending = m_sort_ascending;
    const auto lambda = [ascending](const std::pair<std::string,FrameID> &a, const std::pair<std::string,FrameID> &b) {
        if (a.second.group_number != b.second.group_number) {
            if (ascending) {
                return a.second.group_number < b.second.group_number;
            } else {
                return a.second.group_number > b.second.group_number;
            }
        }
        if (a.second.type != b.second.type) {
            return a.second.type < b.second.type;
        }
        if (ascending) {
            return a.second.input_frame < b.second.input_frame;
        } else {
            return a.second.input_frame > b.second.input_frame;
        }
    };
    std::sort(m_shown_frames.begin(), m_shown_frames.end(), lambda);
};

std::string FilelistHandlerGUIInterface::get_default_alignment_txt_file_name() const {
    std::vector<FrameInfo> checked_frames = get_checked_frames_of_type(FrameType::LIGHT);
    if (checked_frames.empty()) {
        return "alignment.txt";
    }

    const std::string first_address = checked_frames[0].input_frame.get_file_address();
    for (const auto &frame : checked_frames) {
        if (frame.input_frame.get_file_address() != first_address) {
            return "alignment.txt";
        }
    }

    const vector<string> path_parts = AstroPhotoStacker::split_string(first_address, "/");
    if (path_parts.empty()) {
        return "alignment.txt";
    }

    const std::string file_name = path_parts.back();
    return file_name + "_alignment.txt";
};

void FilelistHandlerGUIInterface::sort_frames() {
    switch (m_sort_type) {
        case SortType::NAME:
            sort_by_name_internal();
            break;
        case SortType::RANKING:
            sort_by_ranking_internal();
            break;
        case SortType::GROUP:
            sort_by_group_internal();
            break;
        case SortType::BRIGHTNESS_MEAN:
            sort_by_mean_brightness_internal();
            break;
    }
};