#include "../headers/ImageViewerApp.h"

#include "../../headers/Common.h"
#include "../../headers/raw_file_reader.h"

#include <iostream>
#include <filesystem>

using namespace std;

bool ImageViewerApp::OnInit()    {
    ImageViewerFrame *frame = new ImageViewerFrame();
    frame->Show(true);
    return true;
}


ImageViewerFrame::ImageViewerFrame()
    : wxFrame(nullptr, wxID_ANY, "Image Viewer") {

    m_main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

    // full screen
    SetSize(wxGetDisplaySize());

    add_menu_bar();

    wxBoxSizer *m_sizer_main_frame = new wxBoxSizer(wxHORIZONTAL);

    m_image_preview = make_unique<ImagePreview>(this, 1080, 720, 255, true);

    m_sizer_main_frame->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);

    bind_key_events();

    CreateStatusBar();
    SetStatusText("Author: Michal Dubovsky");

};

void ImageViewerFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}

void ImageViewerFrame::add_menu_bar()   {
    wxMenuBar *menu_bar = new wxMenuBar;

    wxMenu *file_menu = new wxMenu;
    int id = unique_counter();
    file_menu->Append(id, "Open file", "Open file");
    Bind(wxEVT_MENU, &ImageViewerFrame::on_open_file, this, id);

    file_menu->Append(wxID_EXIT);
    Bind(wxEVT_MENU, &ImageViewerFrame::on_exit,  this, wxID_EXIT);


    menu_bar->Append(file_menu, "&File");

    SetMenuBar(menu_bar);
};


void ImageViewerFrame::on_open_file(wxCommandEvent& event)    {
    const std::string default_path = ".";
    string wildcard_string = "Image files |";
    for (const string &extension : m_allowed_extensions) {
        wildcard_string += "*." + extension + ";" + "*." + AstroPhotoStacker::to_upper_copy(extension) + ";";
    }

    wxFileDialog dialog(this, "Open image", "", default_path, wildcard_string, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() == wxID_OK) {
        const std::string file_address = dialog.GetPath().ToStdString();
        open_file(file_address);
    }
    dialog.Destroy();

};

void ImageViewerFrame::open_file(const std::string &file_address)   {
    m_image_preview->read_preview_from_file(file_address);
    m_image_preview->update_preview_bitmap();


    m_file_address = file_address;
};

void ImageViewerFrame::bind_key_events()  {
    // right arrow
    m_main_panel->Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_RIGHT) {
            load_consecutive_file(1);
        }
        else if (event.GetKeyCode() == WXK_LEFT) {
            load_consecutive_file(-1);
        }
    });
};

void ImageViewerFrame::load_consecutive_file(int direction)   {
    std::filesystem::path path = m_file_address;
    std::filesystem::path parent_path = path.parent_path();

    vector<string> files;
    size_t current_file_index = 0;
    size_t i = 0;
    for (const auto &entry : std::filesystem::directory_iterator(parent_path)) {
        if (entry.path().extension() == path.extension()) {
            files.push_back(entry.path().string());
            if (entry.path().string() == m_file_address) {
                current_file_index = i;
            }
            i++;
        }
    }

    size_t next_file_index = (current_file_index + direction) % files.size();
    open_file(files[next_file_index]);
};