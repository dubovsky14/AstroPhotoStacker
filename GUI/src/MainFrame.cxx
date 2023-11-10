#include "../headers/MainFrame.h"

bool MyApp::OnInit()    {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "AstroPhotoStacker GUI") {

    m_sizer_main_frame = new wxBoxSizer(wxVERTICAL);
    m_panel_top     = new wxPanel(this, wxID_ANY);
    m_sizer_main_frame->Add(m_panel_top   , 1,wxEXPAND | wxALL, 5);
    SetSizer(m_sizer_main_frame);

    add_files_to_stack_checkbox();

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
    for (auto file : m_filelist_handler.get_files(FileTypes::LIGHT))   {
        m_files_to_stack_checkbox->Append(file);
    }
    for (auto file : m_filelist_handler.get_files(FileTypes::DARK))   {
        m_files_to_stack_checkbox->Append(file);
    }
    for (auto file : m_filelist_handler.get_files(FileTypes::FLAT))   {
        m_files_to_stack_checkbox->Append(file);
    }
    for (auto file : m_filelist_handler.get_files(FileTypes::BIAS))   {
        m_files_to_stack_checkbox->Append(file);
    }
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
