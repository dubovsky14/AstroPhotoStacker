#include "../headers/AlignmentFrame.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/thread_pool.h"

#include <vector>
#include <iostream>
#include <wx/progdlg.h>

using namespace std;
using namespace AstroPhotoStacker;

AlignmentFrame::AlignmentFrame(MyFrame *parent, FilelistHandler *filelist_handler, StackSettings *stack_settings)
    :  wxFrame(parent, wxID_ANY, "Select alignment file")      {

    //SetSize(400, 200);

    m_stack_settings = stack_settings;
    m_filelist_handler = filelist_handler;
    wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, "Select alignment file:");
    select_file_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    const std::vector<InputFrame>  &all_light_frames_addresses = filelist_handler->get_frames(FileTypes::LIGHT);
    const std::vector<bool>        &light_frames_is_checked    = filelist_handler->get_frames_checked(FileTypes::LIGHT);
    vector<int>     indices_frames_to_align;
    vector<InputFrame>  frames_to_align;

    std::vector<wxString> available_light_frames;
    for (unsigned int i = 0; i < all_light_frames_addresses.size(); ++i) {
        if (light_frames_is_checked[i]) {
            available_light_frames.push_back(all_light_frames_addresses[i].to_string());
            indices_frames_to_align.push_back(i);
            frames_to_align.push_back(all_light_frames_addresses[i]);
        }
    }
    wxChoice* choice_box_alignment_frame = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, available_light_frames.size(), available_light_frames.data());
    choice_box_alignment_frame->SetSelection(0);
    stack_settings->set_alignment_frame(frames_to_align[0]);
    choice_box_alignment_frame->Bind(wxEVT_CHOICE, [choice_box_alignment_frame, stack_settings](wxCommandEvent&){
        int current_selection = choice_box_alignment_frame->GetSelection();
        std::string alignment_frame = choice_box_alignment_frame->GetString(current_selection).ToStdString();
        stack_settings->set_alignment_frame(alignment_frame);
    });


    wxStaticText* select_alignment_method = new wxStaticText(this, wxID_ANY, "Select alignment method:");
    select_alignment_method->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    std::vector<wxString> alignment_methods;
    alignment_methods.push_back("stars");
    alignment_methods.push_back("planetary");
    alignment_methods.push_back("planetary without rotation");
    alignment_methods.push_back("surface");
    wxChoice* choice_box_alignment_method = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, alignment_methods.size(), alignment_methods.data());
    choice_box_alignment_method->SetSelection(0);
    stack_settings->set_alignment_method("stars");
    choice_box_alignment_method->Bind(wxEVT_CHOICE, [choice_box_alignment_method, stack_settings](wxCommandEvent&){
        int current_selection = choice_box_alignment_method->GetSelection();
        std::string alignment_method = choice_box_alignment_method->GetString(current_selection).ToStdString();
        stack_settings->set_alignment_method(alignment_method);
    });

    wxButton* button_ok = new wxButton(this, wxID_ANY, "Align files");
    button_ok->Bind(wxEVT_BUTTON, [this, frames_to_align, indices_frames_to_align, parent](wxCommandEvent&){
        // TODO
        AstroPhotoStacker::PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_alignment_method(m_stack_settings->get_alignment_method());
        photo_alignment_handler.set_number_of_cpu_threads(m_stack_settings->get_n_cpus());
        const std::atomic<int> &n_processed = photo_alignment_handler.get_number_of_aligned_files();

        const int files_total = frames_to_align.size();
        wxProgressDialog *progress_bar = new wxProgressDialog("Aligning files", "Aligned 0 / " + std::to_string(files_total) + " files", files_total, nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        progress_bar->Update(n_processed);

        thread_pool pool(1);
        pool.submit([this, &photo_alignment_handler, &frames_to_align](){
            photo_alignment_handler.align_files(m_stack_settings->get_alignment_frame(), frames_to_align);
        });

        while (pool.get_tasks_total()) {
            progress_bar->Update(n_processed, "Aligned " + std::to_string(n_processed) + " / " + std::to_string(files_total) + " files");
            wxMilliSleep(100);
        }
        progress_bar->Close();

        pool.wait_for_tasks();

        const std::vector<AstroPhotoStacker::FileAlignmentInformation> &alignment_info = photo_alignment_handler.get_alignment_parameters_vector();
        const std::vector<std::vector<AstroPhotoStacker::LocalShift>> &local_shifts = photo_alignment_handler.get_local_shifts_vector();

        for (unsigned int i_selected_file = 0; i_selected_file < indices_frames_to_align.size(); ++i_selected_file) {
            int i_file = indices_frames_to_align[i_selected_file];
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

    main_sizer->Add(select_file_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    main_sizer->Add(choice_box_alignment_frame, 0,  wxEXPAND, 5);
    main_sizer->Add(select_alignment_method, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    main_sizer->Add(choice_box_alignment_method, 0,  wxEXPAND, 5);
    main_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    this->SetSizer(main_sizer);

};