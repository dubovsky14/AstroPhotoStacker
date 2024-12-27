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

struct RowInfo {
    FileTypes   file_type;
    int         file_index;
    int         frame_index;
};
class FramesCheckbox  {
    public:
        FramesCheckbox( wxWindow *parent,
                        const wxArrayString& choices = {},
                        wxWindowID id = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxLB_MULTIPLE);

        void add_sizer(wxBoxSizer *sizer, int proportion, int flag, int border);

        void add_files(const std::vector<std::string> &files, FileTypes file_type);

        std::vector<AstroPhotoStacker::InputFrame> get_checked_frames(FileTypes type) const;

        void add_on_click_callback(const std::function<void(const AstroPhotoStacker::InputFrame&)> &callback);

        void set_checked_status_for_all_frames(bool checked);

        //std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const AstroPhotoStacker::StackSettings &stack_settings) const;

        void set_alignment_info(const AstroPhotoStacker::InputFrame &frame, const AlignmentFileInfo& alignment_info);

        const AlignmentFileInfo get_alignment_info(const AstroPhotoStacker::InputFrame &frame) const;

        AstroPhotoStacker::Metadata get_metadata(const AstroPhotoStacker::InputFrame &frame) const;

        void remove_checked_frames();

        /*
        @brief Updates the checked status of the frames in the filelist based on the checked status of the checkboxes

        // #TODO: Refactor this and move it to private
        */
        void update_checked_elements();


    private:
        wxCheckListBox *m_files_to_stack_checkbox;
        int get_light_file_index(const AstroPhotoStacker::InputFrame &input_frame) const;

        void update_checkbox();


        std::map<FileTypes, std::vector<InputFile>> m_input_files;
        std::map<FileTypes, std::vector<bool>>      m_input_files_are_unrolled;
        std::function<void(const AstroPhotoStacker::InputFrame&)>      m_on_click_callback = nullptr;

        std::vector<RowInfo> m_row_info_vector; // -1 if the row is a file, otherwise the index of the frame in the file
};