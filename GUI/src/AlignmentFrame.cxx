#include "../headers/AlignmentFrame.h"
#include "../headers/ProgressBarWindow.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/thread_pool.h"
#include "../../headers/InputFrame.h"
#include "../../headers/AlignmentPointBox.h"
#include "../../headers/AlignmentSettingsSurface.h"

#include <wx/progdlg.h>

#include <vector>
#include <iostream>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentFrame::AlignmentFrame(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gui_interface, StackSettings *stack_settings, std::vector<AstroPhotoStacker::AlignmentPointBox> *alignment_box_vector_storage)
    :  wxFrame(parent, wxID_ANY, "Select alignment file")      {

    SetSize(m_window_size);

    m_stack_settings = stack_settings;
    m_filelist_handler_gui_interface = filelist_handler_gui_interface;
    m_alignment_box_vector_storage = alignment_box_vector_storage;
    m_main_sizer = new wxBoxSizer(wxVERTICAL);

    initialize_list_of_frames_to_align();

    add_reference_file_selection_menu();

    add_alignment_method_menu();

    add_surface_method_settings();

    add_button_align_files(parent);

    this->SetSizer(m_main_sizer);
};

void AlignmentFrame::initialize_list_of_frames_to_align()   {
    m_indices_frames_to_align.clear();
    m_frames_to_align.clear();
    m_available_light_frames.clear();

    const std::vector<InputFrame>  &all_light_frames_addresses = m_filelist_handler_gui_interface->get_frames(FileTypes::LIGHT);
    const std::vector<bool>        &light_frames_is_checked    = m_filelist_handler_gui_interface->get_frames_checked(FileTypes::LIGHT);
    const unsigned int max_number_of_frames_for_gui = 2000; // without this, it would freeze for planetary videos
    for (unsigned int i = 0; i < all_light_frames_addresses.size(); ++i) {
        if (light_frames_is_checked[i]) {
            if (i < max_number_of_frames_for_gui) {
                m_available_light_frames.push_back(all_light_frames_addresses[i].to_gui_string());
            }
            m_indices_frames_to_align.push_back(i);
            m_frames_to_align.push_back(all_light_frames_addresses[i]);
        }
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
        std::string alignment_frame_gui_string = choice_box_alignment_frame->GetString(current_selection).ToStdString();
        const InputFrame alignment_frame = InputFrame::build_from_gui_string(alignment_frame_gui_string);
        m_stack_settings->set_alignment_frame(alignment_frame);
    });
    m_main_sizer->Add(select_file_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_sizer->Add(choice_box_alignment_frame, 0,  wxEXPAND, 5);
};

void AlignmentFrame::add_alignment_method_menu()    {
    wxStaticText* select_alignment_method = new wxStaticText(this, wxID_ANY, "Select alignment method:");
    select_alignment_method->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    wxChoice* choice_box_alignment_method = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_alignment_methods.size(), m_alignment_methods.data());
    choice_box_alignment_method->SetSelection(0);
    m_stack_settings->set_alignment_method("stars");
    choice_box_alignment_method->Bind(wxEVT_CHOICE, [this, choice_box_alignment_method](wxCommandEvent&){
        int current_selection = choice_box_alignment_method->GetSelection();
        std::string alignment_method = choice_box_alignment_method->GetString(current_selection).ToStdString();
        m_stack_settings->set_alignment_method(alignment_method);
        update_options_visibility(alignment_method);
    });

    m_main_sizer->Add(select_alignment_method, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_sizer->Add(choice_box_alignment_method, 0,  wxEXPAND, 5);
};

void AlignmentFrame::add_surface_method_settings()  {
    m_hidden_options_sizer = new wxBoxSizer(wxVERTICAL);

    AlignmentSettingsSurface *alignment_settings_surface = AlignmentSettingsSurface::get_instance();

    auto add_hidden_settings_slider = [this, alignment_settings_surface](
            const std::string &alignment_method,
            const std::string &label,
            float min_value,
            float max_value,
            float initial_value,
            float step,
            int n_digits,
            std::function<void(float)> callback
        )   {
            if (find(m_alignment_methods.begin(), m_alignment_methods.end(), alignment_method) == m_alignment_methods.end())    {
                throw std::runtime_error("Attempted to use \'add_hidden_settings_slider\' function with unknown alignment method.");
            }

            auto this_slider = make_unique<FloatingPointSlider>(
                    this,
                    label,
                    min_value,
                    max_value,
                    initial_value,
                    step,
                    n_digits,
                    callback
            );
            this_slider->add_sizer(m_hidden_options_sizer, 0, wxEXPAND, 5);
            this_slider->hide();

            if (m_hidden_settings_sliders.find(alignment_method) == m_hidden_settings_sliders.end()) {
                m_hidden_settings_sliders[alignment_method] = vector<unique_ptr<FloatingPointSlider>>();
            }
            m_hidden_settings_sliders[alignment_method].push_back(std::move(this_slider));
    };


    add_hidden_settings_slider(
        "surface",
        "Contrast threshold: ",
        0,
        1,
        AlignmentPointBox::get_contrast_threshold(),
        0.01,
        2,
        [](float value){AlignmentPointBox::set_contrast_threshold(value);
    });

    add_hidden_settings_slider(
        "surface",
        "Number of boxes: ",
        0,
        500,
        alignment_settings_surface->get_number_of_boxes(),
        1,
        0,
        [](float value){
            AlignmentSettingsSurface *alignment_settings_surface = AlignmentSettingsSurface::get_instance();
            alignment_settings_surface->set_number_of_boxes(value+0.1);
    });

    add_hidden_settings_slider(
        "surface",
        "Maximal allowed overlap between alignment boxes: ",
        0,
        1,
        alignment_settings_surface->get_max_overlap_between_boxes(),
        0.01,
        2,
        [](float value){
            AlignmentSettingsSurface *alignment_settings_surface = AlignmentSettingsSurface::get_instance();
            alignment_settings_surface->set_max_overlap_between_boxes(value);
        }
    );



    auto add_hidden_checkbox = [this, alignment_settings_surface](
            const std::string &alignment_method,
            const std::string &label,
            const std::string &tooltip,
            bool default_value,
            std::function<void(bool)> callback)   {
                    if (find(m_alignment_methods.begin(), m_alignment_methods.end(), alignment_method) == m_alignment_methods.end())    {
                        throw std::runtime_error("Attempted to use \'add_hidden_settings_slider\' function with unknown alignment method.");
                    }

                    wxCheckBox* checkbox = new wxCheckBox(this, wxID_ANY, label);
                    checkbox->SetValue(default_value);
                    checkbox->SetToolTip(tooltip);
                    checkbox->Bind(wxEVT_CHECKBOX, [callback, checkbox, this](wxCommandEvent&){
                        const bool is_checked = checkbox->GetValue();
                        callback(is_checked);
                    });
                    m_hidden_options_sizer->Add(checkbox, 0, wxEXPAND, 5);

                    if (m_hidden_settings_checkboxes.find(alignment_method) == m_hidden_settings_checkboxes.end()) {
                        m_hidden_settings_checkboxes[alignment_method] = vector<wxCheckBox*>();
                    }
                    m_hidden_settings_checkboxes[alignment_method].push_back(checkbox);
    };

    add_hidden_checkbox(
        "surface",
        "Use regular grid",
        "If checked, APs will be placed in regular grid. If not checked, their positions will be random.",
        alignment_settings_surface->get_regular_grid(),
        [](bool value){
            AlignmentSettingsSurface *alignment_settings_surface = AlignmentSettingsSurface::get_instance();
            alignment_settings_surface->set_regular_grid(value);
        }
    );



    m_main_sizer->Add(m_hidden_options_sizer, 2, wxEXPAND, 5);

    update_options_visibility(m_stack_settings->get_alignment_method());
};

void AlignmentFrame::update_options_visibility(const std::string &selected_alignment_method)    {
    for (const auto &[available_method, vector_of_sliders] : m_hidden_settings_sliders)    {
        if (wxString(available_method) == selected_alignment_method) {
            for (const unique_ptr<FloatingPointSlider> &slider : vector_of_sliders)  {
                slider->show();
            }
        }
        else {
            for (const unique_ptr<FloatingPointSlider> &slider : vector_of_sliders)  {
                slider->hide();
            }
        }
    }

    for (const auto &[available_method, vector_of_checkboxes] : m_hidden_settings_checkboxes)    {
        if (wxString(available_method) == selected_alignment_method) {
            for (wxCheckBox* checkbox : vector_of_checkboxes)  {
                checkbox->Show();
            }
        }
        else {
            for (wxCheckBox* checkbox : vector_of_checkboxes)  {
                checkbox->Hide();
            }
        }
    }

    m_hidden_options_sizer->Layout();
    m_hidden_options_sizer->Fit(this);
    SetSize(m_window_size);
};

void AlignmentFrame::add_button_align_files(MyFrame *parent)    {
    wxButton* button_ok = new wxButton(this, wxID_ANY, "Align files");
    button_ok->Bind(wxEVT_BUTTON, [this, parent](wxCommandEvent&){
        // TODO
        AstroPhotoStacker::PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_alignment_method(m_stack_settings->get_alignment_method());
        photo_alignment_handler.set_number_of_cpu_threads(m_stack_settings->get_n_cpus());
        if (m_alignment_box_vector_storage != nullptr) {
            m_alignment_box_vector_storage->clear();
            photo_alignment_handler.set_alignment_box_vector_storage(m_alignment_box_vector_storage);
        }


        const int files_total = m_indices_frames_to_align.size();
        const std::atomic<int> &n_processed = photo_alignment_handler.get_number_of_aligned_files();
        run_task_with_progress_dialog(  "Aligning files",
                                        "Aligned ",
                                        "files",
                                        n_processed,
                                        files_total,
                                        [this, &photo_alignment_handler](){photo_alignment_handler.align_files(m_stack_settings->get_alignment_frame(), m_frames_to_align);},
                                        "All files aligned", 100);

        const std::vector<AstroPhotoStacker::FileAlignmentInformation> &alignment_info = photo_alignment_handler.get_alignment_parameters_vector();
        const std::vector<std::vector<AstroPhotoStacker::LocalShift>> &local_shifts = photo_alignment_handler.get_local_shifts_vector();

        for (unsigned int i_selected_file = 0; i_selected_file < m_indices_frames_to_align.size(); ++i_selected_file) {
            int i_file = m_indices_frames_to_align[i_selected_file];
            const AstroPhotoStacker::FileAlignmentInformation &info = alignment_info[i_selected_file];
            AlignmentFileInfo alignment_file_info;
            alignment_file_info.shift_x = info.shift_x;
            alignment_file_info.shift_y = info.shift_y;
            alignment_file_info.rotation_center_x = info.rotation_center_x;
            alignment_file_info.rotation_center_y = info.rotation_center_y;
            alignment_file_info.rotation = info.rotation;
            alignment_file_info.ranking = info.ranking;
            alignment_file_info.initialized = true;
            m_filelist_handler_gui_interface->set_alignment_info(i_file, alignment_file_info);

            const std::vector<AstroPhotoStacker::LocalShift> &shifts = local_shifts[i_selected_file];
            if (shifts.size() > 0) {
                m_filelist_handler_gui_interface->set_local_shifts(i_file, shifts);
            }
        }
        parent->update_files_to_stack_checkbox();
        parent->update_alignment_status();
        this->Close();
    });

    m_main_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};