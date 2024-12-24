#pragma once

#include "../headers/InputFile.h"
#include "../headers/FilelistHandler.h"

#include "../../headers/InputFrame.h"
#include "../../headers/MetadataManager.h"
#include "../../headers/StackerBase.h"
#include "../../headers/StackSettings.h"
#include "../../headers/InputFrame.h"

#include <wx/wx.h>

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <map>


class FramesCheckbox  {
    public:
        FramesCheckbox(wxWindow *parent);

        void add_files(const std::vector<std::string> &files, FileTypes file_type);

        std::vector<AstroPhotoStacker::InputFrame> get_checked_frames(FileTypes type) const;

        void add_on_click_callback(const std::function<void(const AstroPhotoStacker::InputFrame&)> &callback);

        void set_checked_status_for_all_frames(bool checked);

        //std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const AstroPhotoStacker::StackSettings &stack_settings) const;

        void set_alignment_info(const AstroPhotoStacker::InputFrame &frame, const AlignmentFileInfo& alignment_info);

        const AlignmentFileInfo get_alignment_info(const AstroPhotoStacker::InputFrame &frame) const;

        AstroPhotoStacker::Metadata get_metadata(const AstroPhotoStacker::InputFrame &frame) const;

    private:
        wxCheckListBox *m_files_to_stack_checkbox;
        int get_light_file_index(const AstroPhotoStacker::InputFrame &input_frame) const;

        std::map<FileTypes, std::vector<InputFile>> m_input_files;
        std::function<void(const AstroPhotoStacker::InputFrame&)>      m_on_click_callback = nullptr;
};