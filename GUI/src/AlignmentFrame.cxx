#include "../headers/AlignmentFrame.h"

#include <vector>
#include <iostream>

using std::cout, std::endl;

AlignmentFrame::AlignmentFrame(const std::vector<std::string> &available_light_frames, StackSettings *stack_settings)
    :  wxFrame(nullptr, wxID_ANY, "Select alignment file")      {

    SetSize(400, 200);

    wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, "Select alignment file:");
    select_file_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    m_available_files.resize(available_light_frames.size());
    for (unsigned int i = 0; i < available_light_frames.size(); ++i) {
        m_available_files[i] = available_light_frames[i];
    }
    wxChoice* choice_box_alignment_file = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, available_light_frames.size(), m_available_files.data());
    choice_box_alignment_file->SetSelection(0);
    choice_box_alignment_file->Bind(wxEVT_CHOICE, [choice_box_alignment_file, stack_settings](wxCommandEvent&){
        int current_selection = choice_box_alignment_file->GetSelection();
        std::string alignment_file = choice_box_alignment_file->GetString(current_selection).ToStdString();
        stack_settings->set_alignment_file(alignment_file);
    });


    wxButton* button_ok = new wxButton(this, wxID_ANY, "Align files");
    button_ok->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
        cout << "TODO: Align files" << endl;
        this->Close();
    });

    main_sizer->Add(select_file_text, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    main_sizer->Add(choice_box_alignment_file, 1,  wxEXPAND, 5);
    main_sizer->Add(button_ok, 2, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    this->SetSizer(main_sizer);

};