#pragma once

#include "../headers/AlignmentFrame.h"
#include "../headers/ImagePreviewCometSelectionTool.h"

#include "../../headers/InputFrame.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>
#include <memory>
#include <map>

/**
 * @brief Frame (dialog window) for selecting comet positions in frames

*/
class CometSelectionFrame : public wxDialog  {
    public:
        /**
         * @brief Construct a new Comet Selection Frame object
         *
         * @param parent pointer to the parent frame (alignment frame)
         * @param filelist_handler_gio_interface pointer to the filelist handler GUI interface object
         * @param stack_settings pointer to the stack settings object
         */
        CometSelectionFrame(AlignmentFrame *parent, std::map<AstroPhotoStacker::InputFrame, std::pair<float,float>> *comet_positions_storage, std::vector<AstroPhotoStacker::InputFrame> frames_to_select_from);

    private:

        std::map<AstroPhotoStacker::InputFrame, std::pair<float,float>> *m_comet_positions_storage = nullptr;

        std::unique_ptr<ImagePreviewCometSelectionTool> m_image_preview_comet_selection_tool = nullptr;

        std::vector<AstroPhotoStacker::InputFrame> m_frames_to_select_from;
        AstroPhotoStacker::InputFrame m_current_frame;


        wxBoxSizer *m_main_vertical_sizer = nullptr;
        CombinedColorStrecherTool m_color_stretcher; // for exposure correction
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;


        void add_image_preview();

        void add_exposure_correction_spin_ctrl();

        void add_frames_dropdown_menu();

        void add_control_buttons();

        void sort_frames_by_timestamp();

        void update_image_preview(const AstroPhotoStacker::InputFrame &frame);
};
