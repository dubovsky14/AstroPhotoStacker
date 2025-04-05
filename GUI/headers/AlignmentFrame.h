#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/MainFrame.h"
#include "../headers/FloatingPointSlider.h"

#include "../../headers/StackSettings.h"
#include "../../headers/AlignmentPointBox.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>
#include <memory>


/**
 * @brief Frame (dialog window) for selecting the reference photo for alignment

*/
class AlignmentFrame : public wxFrame  {
    public:
        /**
         * @brief Construct a new Alignment Frame object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param filelist_handler_gio_interface pointer to the filelist handler GUI interface object
         * @param stack_settings pointer to the stack settings object
         */
        AlignmentFrame(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gio_interface, AstroPhotoStacker::StackSettings *stack_settings, std::vector<AstroPhotoStacker::AlignmentPointBox> *alignment_box_vector_storage = nullptr);

    private:
        void add_reference_file_selection_menu();

        void add_surface_method_settings();

        void add_alignment_method_menu();

        void update_options_visibility(const std::string &alignment_method);

        void add_button_align_files(MyFrame *parent);

        std::vector<wxString> m_alignment_methods = {"stars", "planetary", "planetary without rotation", "surface"};

        AstroPhotoStacker::StackSettings *m_stack_settings = nullptr;
        FilelistHandlerGUIInterface *m_filelist_handler_gui_interface = nullptr;
        std::vector<AstroPhotoStacker::AlignmentPointBox> *m_alignment_box_vector_storage = nullptr;

        wxBoxSizer *m_hidden_options_sizer = nullptr;
        wxBoxSizer *m_main_sizer = nullptr;

        std::map<std::string, std::vector<std::unique_ptr<FloatingPointSlider>>>    m_hidden_settings_sliders;
        std::map<std::string, std::vector<wxCheckBox*>>                             m_hidden_settings_checkboxes;


        void initialize_list_of_frames_to_align();
        std::vector<int>                            m_indices_frames_to_align;
        std::vector<AstroPhotoStacker::InputFrame>  m_frames_to_align;
        std::vector<wxString>                       m_available_light_frames;

        wxSize m_window_size = wxSize(600, 400);
};
