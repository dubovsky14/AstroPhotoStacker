#pragma once

#include "../headers/Metadata.h"
#include "../headers/FilelistHandler.h"

#include "../../headers/InputFrame.h"

#include <string>
#include <vector>
#include <map>

struct InputFrameInfoGUI {
    AstroPhotoStacker::InputFrame   input_frame;
    bool                            is_checked;
    AlignmentFileInfo               alignment_info;
    AstroPhotoStacker::Metadata     metadata;
    std::string                     gui_string;
};

class InputFile   {
    public:
        explicit InputFile(const std::string &file_path);

        const std::string &get_file_path() const;

        const std::string get_gui_string() const;

        const std::vector<InputFrameInfoGUI> &get_frames_info() const    {
            return m_frames;
        };

        const std::vector<AstroPhotoStacker::InputFrame> get_frames() const;

        const std::vector<AstroPhotoStacker::InputFrame> get_checked_frames() const;

        bool has_multiple_frames() const;

        void set_frame_is_checked(const AstroPhotoStacker::InputFrame &input_frame, bool is_checked);

        bool frame_is_checked(const AstroPhotoStacker::InputFrame &input_frame) const;

        void remove_frame(const AstroPhotoStacker::InputFrame &input_frame);

        bool all_frames_are_aligned() const;

        void set_check_for_all_frames(bool checked);

        void set_alignment_info(const AstroPhotoStacker::InputFrame &input_frame, const AlignmentFileInfo &alignment_info);

        AlignmentFileInfo get_alignment_info(const AstroPhotoStacker::InputFrame &input_frame) const;

        AstroPhotoStacker::Metadata get_metadata(const AstroPhotoStacker::InputFrame &input_frame) const;

    private:
        void add_frame(const AstroPhotoStacker::InputFrame &input_frame, bool is_checked, const AlignmentFileInfo &alignment_info, const AstroPhotoStacker::Metadata &metadata);

        std::vector<InputFrameInfoGUI> m_frames;
        std::string m_file_path;

};