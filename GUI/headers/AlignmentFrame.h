#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/MainFrame.h"
#include "../headers/FloatingPointSlider.h"

#include "../../headers/StackSettings.h"
#include "../headers/ConfigurableAlgorithmSettings.h"

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
        AlignmentFrame(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gio_interface, AstroPhotoStacker::StackSettings *stack_settings);

    private:
        void add_reference_file_selection_menu();

        void add_alignment_method_menu();

        void update_options_visibility(const std::string &alignment_method);

        void add_button_align_files(MyFrame *parent);

        std::vector<wxString> m_alignment_methods = {"stars", "stars with variable zoom", "planetary without rotation", "surface", "comet"};

        AstroPhotoStacker::StackSettings *m_stack_settings = nullptr;
        FilelistHandlerGUIInterface *m_filelist_handler_gui_interface = nullptr;

        wxBoxSizer *m_sizer_algorithm_specific_settings = nullptr;
        wxBoxSizer *m_main_sizer = nullptr;

        std::vector<std::unique_ptr<FloatingPointSlider>>    m_algorithm_settings_sliders;
        std::vector<wxCheckBox*>                             m_algorithm_settings_checkboxes;
        std::vector<wxStaticText*>                           m_algorithm_settings_static_texts;

        AstroPhotoStacker::ConfigurableAlgorithmSettingsMap m_configurable_algorithm_settings_map;


        void initialize_list_of_frames_to_align();
        std::vector<int>                            m_indices_frames_to_align;
        std::vector<AstroPhotoStacker::InputFrame>  m_frames_to_align;
        std::vector<wxString>                       m_available_light_frames;

        wxSize m_window_size = wxSize(600, 400);

        std::string m_selected_alignment_method = "stars";

        template<class ElementType>
        void clear_vector_of_algorithm_settings_elements(std::vector<ElementType*> &elements_vector) {
            for (ElementType* element : elements_vector) {
                m_sizer_algorithm_specific_settings->Detach(element);
                delete element;
            }
            elements_vector.clear();
        };
};
