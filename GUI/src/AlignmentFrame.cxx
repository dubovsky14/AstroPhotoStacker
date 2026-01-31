#include "../headers/AlignmentFrame.h"
#include "../headers/ProgressBarWindow.h"
#include "../headers/CometSelectionFrame.h"
#include "../headers/SettingsCustomization.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/thread_pool.h"
#include "../../headers/InputFrame.h"
#include "../../headers/ReferencePhotoHandlerFactory.h"

#include <wx/progdlg.h>

#include <vector>
#include <iostream>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentFrame::AlignmentFrame(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gui_interface, StackSettings *stack_settings)
    :  wxFrame(parent, wxID_ANY, "Select alignment file")      {

    SetSize(m_window_size);

    m_stack_settings = stack_settings;
    m_filelist_handler_gui_interface = filelist_handler_gui_interface;
    m_main_sizer = new wxBoxSizer(wxVERTICAL);

    const std::vector<std::string> available_methods = ReferencePhotoHandlerFactory::get_available_alignment_methods();
    for (const std::string &method : available_methods) {
        m_alignment_methods.push_back(wxString(method));
    }


    initialize_list_of_frames_to_align();

    add_reference_file_selection_menu();

    add_alignment_method_menu();

    add_button_align_files(parent);

    this->SetSizer(m_main_sizer);
};

void AlignmentFrame::initialize_list_of_frames_to_align()   {
    m_indices_frames_to_align.clear();
    m_frames_to_align.clear();
    m_available_light_frames.clear();

    const std::vector<std::pair<std::string, FrameID>> &all_frames = m_filelist_handler_gui_interface->get_shown_frames();
    std::vector<std::pair<std::string, FrameID>> checked_light_frames;
    for (size_t i = 0; i < all_frames.size(); ++i) {
        const pair<string,FrameID> &frame = all_frames[i];
        if (frame.second.type != FrameType::LIGHT)  continue;
        const bool is_checked = m_filelist_handler_gui_interface->frame_is_checked(i);
        if (is_checked) {
            checked_light_frames.push_back(frame);
        }
    }

    const unsigned int max_number_of_frames_for_gui = 2000; // without this, it would freeze for planetary videos
    const bool show_full_frame_paths = SettingsCustomization::get_instance().other_settings_customization.show_full_frame_paths;
    for (unsigned int i = 0; i < checked_light_frames.size(); ++i) {
        if (i < max_number_of_frames_for_gui) {
            m_available_light_frames.push_back(checked_light_frames[i].second.input_frame.to_gui_string(show_full_frame_paths));
        }
        m_indices_frames_to_align.push_back(i);
        m_frames_to_align.push_back(checked_light_frames[i].second.input_frame);
    }
};

void AlignmentFrame::add_reference_file_selection_menu()    {
    wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, "Select alignment file:");
    select_file_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));


    wxChoice* choice_box_alignment_frame = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_available_light_frames.size(), m_available_light_frames.data());
    choice_box_alignment_frame->SetSelection(0);
    m_stack_settings->set_alignment_frame(m_frames_to_align[0]);
    choice_box_alignment_frame->Bind(wxEVT_CHOICE, [this, choice_box_alignment_frame](wxCommandEvent&){
        int current_selection = choice_box_alignment_frame->GetSelection();
        m_stack_settings->set_alignment_frame(m_frames_to_align[current_selection]);
    });
    m_main_sizer->Add(select_file_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_sizer->Add(choice_box_alignment_frame, 0,  wxEXPAND, 5);
};

void AlignmentFrame::add_alignment_method_menu()    {
    wxStaticText* select_alignment_method = new wxStaticText(this, wxID_ANY, "Select alignment method:");
    select_alignment_method->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    wxChoice* choice_box_alignment_method = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_alignment_methods.size(), m_alignment_methods.data());
    const std::string default_alignment_method = "stars";
    const int default_index = std::distance(m_alignment_methods.begin(), std::find(m_alignment_methods.begin(), m_alignment_methods.end(), wxString(default_alignment_method)));
    choice_box_alignment_method->SetSelection(default_index);
    m_stack_settings->set_alignment_method(default_alignment_method);
    choice_box_alignment_method->Bind(wxEVT_CHOICE, [this, choice_box_alignment_method](wxCommandEvent&){
        int current_selection = choice_box_alignment_method->GetSelection();
        std::string alignment_method = choice_box_alignment_method->GetString(current_selection).ToStdString();
        m_stack_settings->set_alignment_method(alignment_method);
        update_options_visibility(alignment_method);
    });

    m_main_sizer->Add(select_alignment_method, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_sizer->Add(choice_box_alignment_method, 0,  wxEXPAND, 5);


    m_sizer_algorithm_specific_settings = new wxBoxSizer(wxVERTICAL);
    m_main_sizer->Add(m_sizer_algorithm_specific_settings, 2, wxEXPAND, 5);

    update_options_visibility(default_alignment_method);
};

void AlignmentFrame::update_options_visibility(const std::string &selected_alignment_method)    {
    m_selected_alignment_method = selected_alignment_method;

    for (unique_ptr<FloatingPointSlider> &slider_ptr : m_algorithm_settings_sliders) {
        slider_ptr->detach_sizer(m_sizer_algorithm_specific_settings);
    }
    m_algorithm_settings_sliders.clear();

    clear_vector_of_algorithm_settings_elements(m_algorithm_settings_checkboxes);
    clear_vector_of_algorithm_settings_elements(m_algorithm_settings_static_texts);

    m_configurable_algorithm_settings_map = ConfigurableAlgorithmSettingsMap();
    ConfigurableAlgorithmSettings configurable_settings = ReferencePhotoHandlerFactory::get_configurable_algorithm_settings(selected_alignment_method);

    const vector<string> numerical_keys = configurable_settings.get_additional_setting_keys_numerical();

    for (const string &key : numerical_keys) {
        AdditionalStackerSettingNumerical setting = configurable_settings.get_additional_setting_numerical(key);
        m_configurable_algorithm_settings_map.numerical_settings[key] = setting.get_default_value();

        auto this_slider = make_unique<FloatingPointSlider>(
                this,
                key + ": ",
                static_cast<float>(setting.get_range().first),
                static_cast<float>(setting.get_range().second),
                static_cast<float>(setting.get_default_value()),
                static_cast<float>(setting.get_step()),
                -1,
                [this, key](float value){
                    m_configurable_algorithm_settings_map.numerical_settings[key] = static_cast<double>(value);
                }
        );
        this_slider->add_sizer(m_sizer_algorithm_specific_settings, 0, wxEXPAND, 5);
        m_algorithm_settings_sliders.push_back(std::move(this_slider));
    }

    const vector<string> bool_keys = configurable_settings.get_additional_setting_keys_bool();
    for (const string &key : bool_keys) {
        const bool default_value = configurable_settings.get_additional_setting_bool(key).get_default_value();
        m_configurable_algorithm_settings_map.bool_settings[key] = default_value;

        wxCheckBox* checkbox = new wxCheckBox(this, wxID_ANY, key);
        checkbox->SetValue(default_value);
        checkbox->Bind(wxEVT_CHECKBOX, [this, key, checkbox](wxCommandEvent&){
            const bool is_checked = checkbox->GetValue();
            m_configurable_algorithm_settings_map.bool_settings[key] = is_checked;
        });
        m_sizer_algorithm_specific_settings->Add(checkbox, 0, wxEXPAND, 5);
        m_algorithm_settings_checkboxes.push_back(checkbox);
    }


    m_sizer_algorithm_specific_settings->Layout();
    m_sizer_algorithm_specific_settings->Fit(this);
    SetSize(m_window_size);
};

void AlignmentFrame::add_button_align_files(MyFrame *parent)    {
    wxButton* button_ok = new wxButton(this, wxID_ANY, "Align files");
    button_ok->Bind(wxEVT_BUTTON, [this, parent](wxCommandEvent&){
        AstroPhotoStacker::PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_alignment_method(m_stack_settings->get_alignment_method(), m_configurable_algorithm_settings_map);
        photo_alignment_handler.set_number_of_cpu_threads(m_stack_settings->get_n_cpus());

        if (m_stack_settings->get_alignment_method() == "comet") {
            std::map<InputFrame, std::pair<float,float>> comet_positions_storage;
            CometSelectionFrame comet_selection_frame(this, &comet_positions_storage, m_frames_to_align);

            // wait until the comet selection frame is closed
            if (comet_selection_frame.ShowModal() != wxID_OK) {
                wxMessageDialog dialog(this, "The comet alignment process was canceled.", "Help", wxOK | wxICON_INFORMATION);
                dialog.ShowModal();
                return;
            }

            photo_alignment_handler.get_comet_positions_map() = comet_positions_storage;
        }


        const int files_total = m_frames_to_align.size();
        const std::atomic<int> &n_processed = photo_alignment_handler.get_number_of_aligned_files();
        run_task_with_progress_dialog(  "Aligning files",
                                        "Aligned ",
                                        "files",
                                        n_processed,
                                        files_total,
                                        [this, &photo_alignment_handler](){
                                            photo_alignment_handler.align_files(m_stack_settings->get_alignment_frame(), m_frames_to_align);
                                        },
                                        "All files aligned", 100);

        const std::map<InputFrame, std::unique_ptr<AlignmentResultBase>> &alignment_info_map = photo_alignment_handler.get_alignment_results_map();

        for (const auto &[input_frame, alignment_result_ptr] : alignment_info_map) {
            m_filelist_handler_gui_interface->set_alignment_info(input_frame, *alignment_result_ptr);
        }

        parent->update_files_to_stack_checkbox();
        parent->update_alignment_status();
        this->Close();
    });

    m_main_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};