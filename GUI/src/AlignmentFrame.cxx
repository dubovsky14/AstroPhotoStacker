#include "../headers/AlignmentFrame.h"

#include "../../headers/PhotoAlignmentHandler.h"

#include <vector>
#include <iostream>

using namespace std;

AlignmentFrame::AlignmentFrame(FilelistHandler *filelist_handler, StackSettings *stack_settings)
    :  wxFrame(nullptr, wxID_ANY, "Select alignment file")      {

    SetSize(400, 200);

    m_stack_settings = stack_settings;
    m_filelist_handler = filelist_handler;
    wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, "Select alignment file:");
    select_file_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    const std::vector<std::string> &all_light_frames_addresses = filelist_handler->get_files(FileTypes::LIGHT);
    const std::vector<bool>        &light_frames_is_checked    = filelist_handler->get_files_checked(FileTypes::LIGHT);
    vector<int>     indices_files_to_align;
    vector<string>  files_to_align;

    std::vector<wxString> available_light_frames;
    for (unsigned int i = 0; i < all_light_frames_addresses.size(); ++i) {
        if (light_frames_is_checked[i]) {
            available_light_frames.push_back(all_light_frames_addresses[i]);
            indices_files_to_align.push_back(i);
            files_to_align.push_back(all_light_frames_addresses[i]);
        }
    }
    wxChoice* choice_box_alignment_file = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, available_light_frames.size(), available_light_frames.data());
    choice_box_alignment_file->SetSelection(0);
    stack_settings->set_alignment_file(files_to_align[0]);
    choice_box_alignment_file->Bind(wxEVT_CHOICE, [choice_box_alignment_file, stack_settings](wxCommandEvent&){
        int current_selection = choice_box_alignment_file->GetSelection();
        std::string alignment_file = choice_box_alignment_file->GetString(current_selection).ToStdString();
        stack_settings->set_alignment_file(alignment_file);
    });


    wxButton* button_ok = new wxButton(this, wxID_ANY, "Align files");
    button_ok->Bind(wxEVT_BUTTON, [this, files_to_align, indices_files_to_align](wxCommandEvent&){
        // TODO
        AstroPhotoStacker::PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_number_of_cpu_threads(m_stack_settings->get_n_cpus());
        photo_alignment_handler.align_files(m_stack_settings->get_alignment_file(), files_to_align);

        const std::vector<AstroPhotoStacker::FileAlignmentInformation> &alignment_info = photo_alignment_handler.get_alignment_parameters_vector();

        for (unsigned int i_selected_file = 0; i_selected_file < indices_files_to_align.size(); ++i_selected_file) {
            int i_file = indices_files_to_align[i_selected_file];
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
        }

        this->Close();
    });

    main_sizer->Add(select_file_text, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    main_sizer->Add(choice_box_alignment_file, 1,  wxEXPAND, 5);
    main_sizer->Add(button_ok, 2, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    this->SetSizer(main_sizer);

};