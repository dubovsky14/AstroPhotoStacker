#include "../headers/ImageViewerApp.h"

#include "../../headers/Common.h"
#include "../../headers/raw_file_reader.h"

using namespace std;

bool ImageViewerApp::OnInit()    {
    ImageViewerFrame *frame = new ImageViewerFrame();
    frame->Show(true);
    return true;
}


ImageViewerFrame::ImageViewerFrame()
    : wxFrame(nullptr, wxID_ANY, "Image Viewer") {

    // full screen
    SetSize(wxGetDisplaySize());

    add_menu_bar();

    wxBoxSizer *m_sizer_main_frame = new wxBoxSizer(wxHORIZONTAL);

    m_image_preview = make_unique<ImagePreview>(this, 1080, 720, 255, true);

    m_sizer_main_frame->Add(m_image_preview->get_image_preview_bitmap(), 1, wxTOP, 0);



    CreateStatusBar();
    SetStatusText("Author: Michal Dubovsky");

};

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
    const std::vector<string> allowed_extensions = {"cr2", "cr3", "jpg", "jpeg", "png", "fit", "tif", "tiff", ".png"};
    string wildcard_string = "Image files |";
    for (const string &extension : allowed_extensions) {
        wildcard_string += "*." + extension + ";" + "*." + AstroPhotoStacker::to_upper_copy(extension) + ";";
    }

    wxFileDialog dialog(this, "Open image", "", default_path, wildcard_string, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() == wxID_OK) {
        const std::string file_address = dialog.GetPath().ToStdString();
        m_image_preview->read_preview_from_file(file_address);
        m_image_preview->update_preview_bitmap();
    }
    dialog.Destroy();

};


void ImageViewerFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}