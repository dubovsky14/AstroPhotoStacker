#include "../headers/MainFrame.h"

#include <wx/spinctrl.h>

#include <iostream>

using namespace std;

bool MyApp::OnInit()    {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "AstroPhotoStacker GUI") {

    m_sizer_main_frame = new wxBoxSizer(wxVERTICAL);
    //m_panel_top     = new wxPanel(this, wxID_ANY);
    //m_sizer_main_frame->Add(m_panel_top   , 1,wxEXPAND | wxALL, 5);
    m_sizer_top         = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_button_bar  = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_main_frame->Add(m_sizer_top,        9, wxEXPAND | wxALL, 5);
    m_sizer_main_frame->Add(m_sizer_button_bar, 1, wxEXPAND | wxALL, 5);

    m_sizer_top_left    = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_center  = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_right   = new wxBoxSizer(wxVERTICAL);

    m_sizer_top->Add(m_sizer_top_left,   1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_center, 1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_right,  1, wxEXPAND | wxALL, 5);

    SetSizer(m_sizer_main_frame);
    //SetSizer(m_sizer_top);

    add_files_to_stack_checkbox();

    add_button_bar();

    add_stack_settings_preview();
    add_image_preview();
    add_image_settings();

    add_menu_bar();

    CreateStatusBar();
    SetStatusText("Author: Michal Dubovsky");
}

void MyFrame::add_file_menu()  {
    m_file_menu = new wxMenu;

    int id = unique_counter();
    m_file_menu->Append(id, "Open light frames", "Open light frames");
    Bind(wxEVT_MENU, &MyFrame::on_open_lights, this, id);

    id = unique_counter();
    m_file_menu->Append(id, "Open flat frames", "Open flat frames");
    Bind(wxEVT_MENU, &MyFrame::on_open_flats, this, id);

    m_file_menu->Append(wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::on_exit,  this, wxID_EXIT);

    m_menu_bar->Append(m_file_menu, "&File");

};

void MyFrame::add_menu_bar()    {
    m_menu_bar = new wxMenuBar;

    add_file_menu();

    SetMenuBar(m_menu_bar);
};

void MyFrame::add_files_to_stack_checkbox()  {
    wxArrayString files;
    m_files_to_stack_checkbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, files, wxLB_MULTIPLE);
    m_sizer_main_frame->Add(m_files_to_stack_checkbox, 9,wxEXPAND | wxALL, 5);
};

void MyFrame::update_files_to_stack_checkbox()   {
    m_files_to_stack_checkbox->Clear();
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        for (auto file : m_filelist_handler.get_files(type))   {
            m_files_to_stack_checkbox->Append(to_string(type) + "\t\t" + file);
        }
    }
};

void MyFrame::add_button_bar()   {
    wxButton *button_align_files    = new wxButton(this, wxID_ANY, "Align files");
    wxButton *button_hot_pixel_id   = new wxButton(this, wxID_ANY, "Identify hot pixels");
    wxButton *button_rank_files     = new wxButton(this, wxID_ANY, "Rank files");
    wxButton *button_stack_files    = new wxButton(this, wxID_ANY, "Stack files");

    button_align_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
        cout << "Align files" << endl;
    });

    button_hot_pixel_id->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
        cout << "Identify hot pixels" << endl;
    });

    button_rank_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
        cout << "Rank files" << endl;
    });


    button_stack_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
        cout << "stack files" << endl;
    });

    m_sizer_button_bar->Add(button_align_files, 1, wxALL, 5);
    m_sizer_button_bar->Add(button_hot_pixel_id,1, wxALL, 5);
    m_sizer_button_bar->Add(button_rank_files,  1, wxALL, 5);
    m_sizer_button_bar->Add(button_stack_files, 1, wxALL, 5);
};

void MyFrame::add_stack_settings_preview()   {
    add_n_cpu_slider();
    add_max_memory_spin_ctrl();
    add_stacking_algorithm_choice_box();
};

void MyFrame::add_n_cpu_slider()    {
    const int max_cpu = m_stack_settings.get_max_threads();

    // Create a wxStaticText to display the current value
    wxStaticText* n_cpu_text = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("Number of CPUs: %d"), max_cpu));

    // Create the wxSlider
    wxSlider* slider_ncpu = new wxSlider(this, wxID_ANY, max_cpu, 1, max_cpu, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_ncpu->Bind(wxEVT_SLIDER, [n_cpu_text, slider_ncpu, this](wxCommandEvent&){
        int current_value = slider_ncpu->GetValue();
        (this->m_stack_settings).set_n_cpus(current_value);
        n_cpu_text->SetLabel(wxString::Format(wxT("Number of CPUs: %d"), current_value));
    });

    // Add the controls to a sizer
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_left->Add(n_cpu_text, 0,   wxEXPAND, 5);
    m_sizer_top_left->Add(slider_ncpu, 0,  wxEXPAND, 5);
};

void MyFrame::add_stacking_algorithm_choice_box()  {
    wxStaticText* stacking_algorithm_text = new wxStaticText(this, wxID_ANY, "Stacking algorithm:");

    wxString stacking_algorithms[m_stack_settings.get_stacking_algorithms().size()];
    for (unsigned int i = 0; i < m_stack_settings.get_stacking_algorithms().size(); ++i) {
        stacking_algorithms[i] = m_stack_settings.get_stacking_algorithms()[i];
    }
    wxChoice* choice_box_stacking_algorithm = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_stack_settings.get_stacking_algorithms().size(), stacking_algorithms);
    choice_box_stacking_algorithm->SetSelection(0);
    choice_box_stacking_algorithm->Bind(wxEVT_CHOICE, [choice_box_stacking_algorithm, this](wxCommandEvent&){
        int current_selection = choice_box_stacking_algorithm->GetSelection();
        (this->m_stack_settings).set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());
    });
    m_sizer_top_left->Add(stacking_algorithm_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(choice_box_stacking_algorithm, 0,  wxEXPAND, 5);
};

void MyFrame::add_max_memory_spin_ctrl() {
    wxStaticText* memory_usage_text = new wxStaticText(this, wxID_ANY, "Maximum memory usage (MB):");

    wxSpinCtrl* spin_ctrl_max_memory = new wxSpinCtrl(this, wxID_ANY, "8000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000, 8000);
    spin_ctrl_max_memory->Bind(wxEVT_SPINCTRL, [spin_ctrl_max_memory, this](wxCommandEvent&){
        int current_value = spin_ctrl_max_memory->GetValue();
        (this->m_stack_settings).set_max_memory(current_value);
    });
    m_sizer_top_left->Add(memory_usage_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(spin_ctrl_max_memory, 0,  wxEXPAND, 5);
};

void MyFrame::add_image_settings()   {
    // TODO
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    m_sizer_top_right->Add(panel, 1, wxEXPAND, 0);
};

void MyFrame::add_image_preview()    {
    // Create a wxImage
    wxImage image(256, 256);

    for (int x = 0; x < 256; ++x) {
        for (int y = 0; y < 256; ++y) {
            image.SetRGB(x, y, 0,0,0);
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Create a wxStaticBitmap to display the image
    wxStaticBitmap* staticBitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

    // Add the wxStaticBitmap to a sizer
    m_sizer_top_center->Add(staticBitmap, 1, wxEXPAND | wxALL, 0);
};

void MyFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}

void MyFrame::on_open_frames(wxCommandEvent& event, FileTypes type, const std::string& title)    {
    wxFileDialog dialog(this, title, "", "", "*[!'.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE | wxFD_CHANGE_DIR);
    if (dialog.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dialog.GetPaths(paths);
        for (auto path : paths) {
            m_filelist_handler.add_file(path.ToStdString(), type);
        }
    }
    dialog.Destroy();
    update_files_to_stack_checkbox();
};

void MyFrame::on_open_lights(wxCommandEvent& event)    {
    on_open_frames(event, FileTypes::LIGHT, "Open light frames");
}

void MyFrame::on_open_flats(wxCommandEvent& event)    {
    on_open_frames(event, FileTypes::FLAT, "Open flat frames");
}
