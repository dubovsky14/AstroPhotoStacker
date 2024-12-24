#include "../headers/InputFile.h"

#include "../../headers/MetadataManager.h"

#include <string>
#include <vector>

using namespace AstroPhotoStacker;
using namespace std;

InputFile::InputFile(const std::string &file_path)  {
    m_file_path = file_path;
    MetadataManager metadata_manager;
    AlignmentFileInfo default_alignment_info;
    if (is_valid_video_file(file_path))   {
        const std::vector<InputFrame> video_frames = get_video_frames(file_path);
        for (const InputFrame &video_frame : video_frames)   {
            const Metadata metadata = metadata_manager.get_metadata(video_frame);
            add_frame(video_frame, false, default_alignment_info, metadata);
        }
    }
    else    {
        const Metadata metadata = metadata_manager.get_metadata(InputFrame(file_path));
        add_frame(InputFrame(file_path), false, default_alignment_info, metadata);
    }
};

const std::string& InputFile::get_file_path() const {
    return m_file_path;
};

const std::vector<AstroPhotoStacker::InputFrame> InputFile::get_frames() const {
    vector<InputFrame> result;
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        result.push_back(frame_info.input_frame);
    }
    return result;
};

const std::vector<AstroPhotoStacker::InputFrame> InputFile::get_checked_frames() const  {
    vector<InputFrame> result;
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.is_checked) {
            result.push_back(frame_info.input_frame);
        }
    }
    return result;
};

bool InputFile::has_multiple_frames() const {
    return m_frames.size() > 1;
};

bool InputFile::frame_is_checked(const AstroPhotoStacker::InputFrame &input_frame) const    {
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.input_frame == input_frame) {
            return frame_info.is_checked;
        }
    }
    return false;
};

void InputFile::set_frame_is_checked(const AstroPhotoStacker::InputFrame &input_frame, bool is_checked)    {
    for (InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.input_frame == input_frame) {
            frame_info.is_checked = is_checked;
        }
    }
};

void InputFile::remove_frame(const AstroPhotoStacker::InputFrame &input_frame)  {
    for (unsigned int i_frame = 0; i_frame < m_frames.size(); ++i_frame) {
        if (m_frames[i_frame].input_frame == input_frame) {
            m_frames.erase(m_frames.begin() + i_frame);
            return;
        }
    }
};

bool InputFile::all_frames_are_aligned() const  {
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        if (!frame_info.alignment_info.initialized) {
            return false;
        }
    }
    return true;
};

void InputFile::set_check_for_all_frames(bool checked)  {
    for (InputFrameInfoGUI &frame_info : m_frames) {
        frame_info.is_checked = checked;
    }
};


void InputFile::set_alignment_info(const AstroPhotoStacker::InputFrame &input_frame, const AlignmentFileInfo &alignment_info)   {
    for (InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.input_frame == input_frame) {
            frame_info.alignment_info = alignment_info;
        }
    }
};

AlignmentFileInfo InputFile::get_alignment_info(const AstroPhotoStacker::InputFrame &input_frame) const {
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.input_frame == input_frame) {
            return frame_info.alignment_info;
        }
    }
    return AlignmentFileInfo();
};

AstroPhotoStacker::Metadata InputFile::get_metadata(const AstroPhotoStacker::InputFrame &input_frame) const {
    for (const InputFrameInfoGUI &frame_info : m_frames) {
        if (frame_info.input_frame == input_frame) {
            return frame_info.metadata;
        }
    }
    return Metadata();
};

void InputFile::add_frame(  const AstroPhotoStacker::InputFrame &input_frame,
                            bool is_checked,
                            const AlignmentFileInfo &alignment_info,
                            const AstroPhotoStacker::Metadata &metadata)    {
    InputFrameInfoGUI frame_info;
    frame_info.input_frame = input_frame;
    frame_info.is_checked = is_checked;
    frame_info.alignment_info = alignment_info;
    frame_info.metadata = metadata;
    m_frames.push_back(frame_info);
};
