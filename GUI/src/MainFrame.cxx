#include "../headers/MainFrame.h"

bool MyApp::OnInit()    {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "AstroPhotoStacker GUI") {

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

void MyFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}

void MyFrame::on_open_lights(wxCommandEvent& event)    {
    wxFileDialog *dialog = new wxFileDialog(this, "Open light frames", "", "", "*[!'txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE | wxFD_CHANGE_DIR);
    if (dialog->ShowModal() == wxID_OK) {
        wxArrayString paths;
        dialog->GetPaths(paths);
        for (auto path : paths) {
            m_filelist_handler.add_file(path.ToStdString(), FileTypes::LIGHT);
            std::cout << path << std::endl;
        }
    }
    dialog->Destroy();
}

void MyFrame::on_open_flats(wxCommandEvent& event)    {
    return;
}
