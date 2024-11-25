#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/MainFrame.h"

#include "../../headers/StackSettings.h"
#include "../../headers/AlignmentPointBox.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>


/**
 * @brief Frame (dialog window) for selecting the reference photo for alignment

*/
class AlignmentFrame : public wxFrame  {
    public:
        /**
         * @brief Construct a new Alignment Frame object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param filelist_handler pointer to the filelist handler object
         * @param stack_settings pointer to the stack settings object
         */
        AlignmentFrame(MyFrame *parent, FilelistHandler *filelist_handler, AstroPhotoStacker::StackSettings *stack_settings, std::vector<AstroPhotoStacker::AlignmentPointBox> *alignment_box_vector_storage = nullptr);

    private:

        void add_surface_method_settings();

        void update_options_visibility(const std::string &alignment_method);

        AstroPhotoStacker::StackSettings *m_stack_settings = nullptr;
        FilelistHandler *m_filelist_handler = nullptr;
        std::vector<AstroPhotoStacker::AlignmentPointBox> *m_alignment_box_vector_storage = nullptr;

        wxBoxSizer *m_hidden_options_sizer = nullptr;
        wxBoxSizer *m_main_sizer = nullptr;

        wxStaticText    *m_contrast_threshold_text = nullptr;
        wxSlider        *m_slider_contrast_threshold = nullptr;

        wxSize m_window_size = wxSize(600, 400);
};
