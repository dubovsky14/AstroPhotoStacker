#include "../headers/SummaryYamlCreator.h"
#include "../../headers/Common.h"

#include <fstream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


const std::string SummaryYamlCreator::s_indent = "  ";
const std::string SummaryYamlCreator::s_indent_new_item = "-" + s_indent.substr(1);

SummaryYamlCreator::SummaryYamlCreator(const FilelistHandlerGUIInterface &filelist_handler_gui) :
    m_filelist_handler_gui(filelist_handler_gui) {

        m_group_numbers = m_filelist_handler_gui.get_group_numbers();
};

std::string SummaryYamlCreator::get_yaml_summary() const{

    string result = block_to_string(get_overall_frames_summary(), "");
    result += "groups:\n";

    for (int group_number : m_group_numbers)   {
        result += block_to_string(get_group_summary(group_number), "");
    }

    /*
    const std::map<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>> &frames_list = m_filelist_handler_gui.get_frames_list();
    for (const std::pair<int, std::map<FrameType, std::map<AstroPhotoStacker::InputFrame,FrameInfo>>> &group_frames : frames_list)    {
        int group_number = group_frames.first;
        result += s_indent + s_indent_new_item + "group_number: " + to_string(group_number) + "\n";
        for (FrameType frame_type : s_file_types_ordering)   {
            const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames_map = group_frames.second.at(frame_type);
            std::string subgroup_filelist = "";

        }

    }

    const AstroPhotoStacker::Metadata &metadata = frame_info.metadata;
    const AlignmentFileInfo &alignment_info     = frame_info.alignment_info;
    const float alignment_score                 = alignment_info.ranking;
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
*/

    return result;
};


std::vector<std::string> SummaryYamlCreator::get_overall_frames_summary() const  {
    std::vector<std::string> result;
    result.push_back("total_frames:");
    for (FrameType frame_type : FilelistHandler::s_file_types_ordering)   {
        const int n_frames = m_filelist_handler_gui.get_number_of_checked_frames(frame_type);
        if (n_frames == 0)   {
            continue;
        }
        result.push_back(s_indent +  to_string(frame_type) + ": " + to_string(n_frames));
    }
    return result;
};


void SummaryYamlCreator::create_and_save_yaml_file(const std::string &output_address) const {
    const std::string yaml_summary = get_yaml_summary();
    std::ofstream output_file(output_address);
    output_file << yaml_summary;
    output_file.close();
};

std::vector<std::string> SummaryYamlCreator::get_group_summary(int group_number) const   {
    std::vector<std::string> result;
    result.push_back(s_indent_new_item +  "group_number: " + to_string(group_number));
    for (FrameType frame_type : FilelistHandler::s_file_types_ordering)   {
        const std::vector<std::string> group_and_type_summary = get_group_and_type_summary(group_number, frame_type);
        if (group_and_type_summary.empty())   {
            continue;
        }

        for (const std::string &line : group_and_type_summary)   {
            result.push_back(s_indent + line);
        }
    }

    return result;
};

std::vector<std::string> SummaryYamlCreator::get_group_and_type_summary(int group_number, FrameType frame_type) const    {
    if (std::find(m_group_numbers.begin(), m_group_numbers.end(), group_number) == m_group_numbers.end())   {
        return {};
    }

    const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames_map = m_filelist_handler_gui.get_frames(frame_type, group_number);
    if (frames_map.empty())   {
        return {};
    }

    std::vector<std::string> result({s_indent_new_item + "frame_type: " + to_string(frame_type)});
    int n_frames = 0;
    for (const auto &frame : frames_map)   {
        const bool is_checked = frame.second.is_checked;
        if (!is_checked)   {
            continue;
        }
        n_frames++;
        result.push_back(s_indent + s_indent_new_item + "file: \"" + frame.first.get_file_address() + "\"");

        int frame_number = frame.first.get_frame_number();
        if (frame_number >= 0) {
            result.push_back(2*s_indent + "frame_number: " + to_string(frame_number));
        }

        const Metadata &metadata = frame.second.metadata;
        if (metadata.aperture > 0) {
            result.push_back(2*s_indent + "aperture: " + AstroPhotoStacker::round_and_convert_to_string(metadata.aperture));
        }
        if (metadata.exposure_time > 0) {
            const std::string exposure_string = metadata.exposure_time > 0.5 ?
                                                AstroPhotoStacker::round_and_convert_to_string(metadata.exposure_time) + " s" :
                                                AstroPhotoStacker::round_and_convert_to_string(metadata.exposure_time * 1000) + " ms";
            result.push_back(2*s_indent + "exposure_time: \"" + exposure_string + "\"");
        }
        if (metadata.iso > 0) {
            result.push_back(2*s_indent + "iso: " + to_string(metadata.iso));
        }
        if (metadata.focal_length > 0) {
            result.push_back(2*s_indent + "focal_length: " + AstroPhotoStacker::round_and_convert_to_string(metadata.focal_length));
        }
        if (metadata.camera_model != "") {
            result.push_back(2*s_indent + "camera_model: \"" + metadata.camera_model + "\"");
        }
        if (metadata.date_time != "") {
            result.push_back(2*s_indent + "date_time: \"" + metadata.date_time + "\"");
        }

    }
    if (n_frames == 0 || result.size() == 0)   {
        return {};
    }
    result.insert(result.begin() + 1, s_indent + "number_of_frames: " + to_string(n_frames));
    return result;
};

std::string SummaryYamlCreator::block_to_string(const std::vector<std::string> &block, const std::string &indent) const {
    std::string result;
    for (const auto &line : block) {
        result += indent + line + "\n";
    }
    return result;
};