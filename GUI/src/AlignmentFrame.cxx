#include "../headers/AlignmentFrame.h"
#include "../headers/ProgressBarWindow.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/thread_pool.h"
#include "../../headers/InputFrame.h"
#include "../../headers/AlignmentPointBox.h"
#include "../../headers/AlignmentSettingsSurface.h"

#include <vector>
#include <iostream>
#include <wx/progdlg.h>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentFrame::AlignmentFrame(MyFrame *parent, FilelistHandler *filelist_handler, StackSettings *stack_settings, std::vector<AstroPhotoStacker::AlignmentPointBox> *alignment_box_vector_storage)
    :  wxFrame(parent, wxID_ANY, "Select alignment file")      {

    SetSize(m_window_size);

    m_stack_settings = stack_settings;
    m_filelist_handler = filelist_handler;
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

    const std::vector<InputFrame>  &all_light_frames_addresses = m_filelist_handler->get_frames(FileTypes::LIGHT);
    const std::vector<bool>        &light_frames_is_checked    = m_filelist_handler->get_frames_checked(FileTypes::LIGHT);

    for (unsigned int i = 0; i < all_light_frames_addresses.size(); ++i) {
        if (light_frames_is_checked[i]) {
            m_available_light_frames.push_back(all_light_frames_addresses[i].to_gui_string());
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
    std::vector<wxString> alignment_methods;
    alignment_methods.push_back("stars");
    alignment_methods.push_back("planetary");
    alignment_methods.push_back("planetary without rotation");
    alignment_methods.push_back("surface");
    wxChoice* choice_box_alignment_method = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, alignment_methods.size(), alignment_methods.data());
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

    m_contrast_threshold_slider = make_unique<FloatingPointSlider>(
        this,
        "Contrast threshold: ",
        0,
        1,
        AlignmentPointBox::get_contrast_threshold(),
        0.01,
        2,
        [](float value){AlignmentPointBox::set_contrast_threshold(value);
    });
    m_contrast_threshold_slider->add_sizer(m_hidden_options_sizer, 0, wxEXPAND, 5);
    m_contrast_threshold_slider->hide();

    m_number_of_boxes_slider = make_unique<FloatingPointSlider>(
        this,
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
    m_number_of_boxes_slider->add_sizer(m_hidden_options_sizer, 0, wxEXPAND, 5);
    m_number_of_boxes_slider->hide();

    m_maximal_overlap_between_boxes_slider = make_unique<FloatingPointSlider>(
        this,
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
    m_maximal_overlap_between_boxes_slider->add_sizer(m_hidden_options_sizer, 0, wxEXPAND, 5);
    m_maximal_overlap_between_boxes_slider->hide();


    m_main_sizer->Add(m_hidden_options_sizer, 2, wxEXPAND, 5);
};

void AlignmentFrame::update_options_visibility(const std::string &alignment_method)    {
    if (alignment_method == "surface") {
        m_contrast_threshold_slider->show();
        m_number_of_boxes_slider->show();
        m_maximal_overlap_between_boxes_slider->show();
    }
    else    {
        m_contrast_threshold_slider->hide();
        m_number_of_boxes_slider->hide();
        m_maximal_overlap_between_boxes_slider->hide();
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
            m_filelist_handler->set_alignment_info(i_file, alignment_file_info);

            const std::vector<AstroPhotoStacker::LocalShift> &shifts = local_shifts[i_selected_file];
            if (shifts.size() > 0) {
                m_filelist_handler->set_local_shifts(i_file, shifts);
            }
        }
        parent->update_files_to_stack_checkbox();
        parent->update_alignment_status();
        this->Close();
    });

    m_main_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};