#include "../headers/MainFrame.h"

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
    m_sizer_top     = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_main_frame->Add(m_sizer_top   , 1,wxEXPAND | wxALL, 5);

    m_sizer_top_left    = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_center  = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_right   = new wxBoxSizer(wxVERTICAL);

    m_sizer_top->Add(m_sizer_top_left,   1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_center, 1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_right,  1, wxEXPAND | wxALL, 5);

    SetSizer(m_sizer_main_frame);
    //SetSizer(m_sizer_top);

    add_files_to_stack_checkbox();

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
    m_sizer_main_frame->Add(m_files_to_stack_checkbox, 1,wxEXPAND | wxALL, 5);
};

void MyFrame::update_files_to_stack_checkbox()   {
    m_files_to_stack_checkbox->Clear();
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        for (auto file : m_filelist_handler.get_files(type))   {
            m_files_to_stack_checkbox->Append(to_string(type) + "\t\t" + file);
        }
    }
};

void MyFrame::add_stack_settings_preview()   {
    n_cpu_slider();
    return;
    // nCPU slider
    int max_cpu = m_stack_settings.get_max_threads();
    wxSlider *slider_ncpu = new wxSlider(this, wxID_ANY, max_cpu, 1, max_cpu, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    m_sizer_top_left->Add(slider_ncpu, 1, wxEXPAND , 0);
};

void MyFrame::n_cpu_slider()    {
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
