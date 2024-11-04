#include "../headers/ImageViewerApp.h"
#include "../headers/Common.h"

#include "../../headers/Common.h"
#include "../../headers/raw_file_reader.h"
#include "../../headers/MetadataReader.h"
#include "../../headers/InputFrame.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

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

    m_sizer_main_frame = new wxBoxSizer(wxVERTICAL);
    m_preview_and_metadata_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_main_frame->Add(m_preview_and_metadata_sizer, 1, wxEXPAND | wxALL, 5);

    m_image_preview = make_unique<ImagePreview>(this, 1080, 720, 255, true);
    m_image_preview->set_exposure_correction(1);

    m_preview_and_metadata_sizer->Add(m_image_preview->get_image_preview_bitmap(), 7, wxEXPAND | wxALL, 5);

    add_metadata_panel();

    add_bottom_panel();

    bind_key_events();

    CreateStatusBar();
    SetSizer(m_sizer_main_frame);
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

void ImageViewerFrame::add_metadata_panel()   {
    const int metadata_text_size = 12;
    wxSizer *metadata_sizer = new wxBoxSizer(wxVERTICAL);
    m_preview_and_metadata_sizer->Add(metadata_sizer, 2, wxCENTER, 5);

    m_metadata_text_exposure = new wxStaticText(this, wxID_ANY, "Exposure: ");
    set_text_size(m_metadata_text_exposure, metadata_text_size);
    metadata_sizer->Add(m_metadata_text_exposure, 0, wxLEFT, 5);

    m_metadata_text_iso = new wxStaticText(this, wxID_ANY, "ISO: ");
    set_text_size(m_metadata_text_iso, metadata_text_size);
    metadata_sizer->Add(m_metadata_text_iso, 0, wxLEFT, 5);

    m_metadata_text_focal_length = new wxStaticText(this, wxID_ANY, "Focal length: ");
    set_text_size(m_metadata_text_focal_length, metadata_text_size);
    metadata_sizer->Add(m_metadata_text_focal_length, 0, wxLEFT, 5);

    m_metadata_text_f_number = new wxStaticText(this, wxID_ANY, "F number: f/");
    set_text_size(m_metadata_text_f_number, metadata_text_size);
    metadata_sizer->Add(m_metadata_text_f_number, 0, wxLEFT, 5);

    m_metadata_text_date_time = new wxStaticText(this, wxID_ANY, "Datetime:");
    set_text_size(m_metadata_text_date_time, metadata_text_size);
    metadata_sizer->Add(m_metadata_text_date_time, 0, wxLEFT, 5);
};

void ImageViewerFrame::update_metadata(const Metadata &metadata)  {
    const string exposure_value_string = metadata.exposure_time > 0.1 ?
                                    AstroPhotoStacker::round_and_convert_to_string(metadata.exposure_time, 2) :
                                    "1/"s + AstroPhotoStacker::round_and_convert_to_string(int(1./metadata.exposure_time), 0);

    m_metadata_text_exposure->SetLabel("Exposure: " + exposure_value_string + " s");
    m_metadata_text_iso->SetLabel("ISO: " + to_string(metadata.iso));
    m_metadata_text_focal_length->SetLabel("Focal length: " + AstroPhotoStacker::round_and_convert_to_string(metadata.focal_length, 1) + " mm");
    m_metadata_text_f_number->SetLabel("F number: f/" + AstroPhotoStacker::round_and_convert_to_string(metadata.aperture, 1));
    m_metadata_text_date_time->SetLabel("Date and time: " + AstroPhotoStacker::unix_time_to_string(metadata.timestamp));
};


void ImageViewerFrame::on_open_file(wxCommandEvent& event)    {
    string wildcard_string = "Image files |";
    for (const string &extension : m_allowed_extensions) {
        wildcard_string += "*." + extension + ";" + "*." + AstroPhotoStacker::to_upper_copy(extension) + ";";
    }

    wxFileDialog dialog(this, "Open image", "", m_current_folder, wildcard_string, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() == wxID_OK) {
        const std::string file_address = dialog.GetPath().ToStdString();
        open_file(file_address);
    }
    dialog.Destroy();

};

void ImageViewerFrame::open_file(const std::string &file_address)   {
    m_image_preview->read_preview_from_file(file_address);
    m_image_preview->update_preview_bitmap();
    m_file_address_text->SetLabel(file_address);

    try {
        const Metadata metadata = read_metadata(InputFrame(file_address));
        update_metadata(metadata);
    }
    catch (const std::exception &e) {
        cout << e.what() << endl;
    }

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

void ImageViewerFrame::add_bottom_panel()   {
    wxBoxSizer *bottom_panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_main_frame->Add(bottom_panel_sizer, 0, wxEXPAND | wxALL, 5);
    m_file_address_text = new wxStaticText(this, wxID_ANY, "");
    set_text_size(m_file_address_text, 12);
    bottom_panel_sizer->Add(m_file_address_text, 0, wxLEFT, 5);
}

void ImageViewerFrame::load_consecutive_file(int direction)   {
    std::filesystem::path path = m_file_address;
    std::filesystem::path parent_path = path.parent_path();
    m_current_folder = parent_path.string();

    vector<string> files;
    for (const auto &entry : std::filesystem::directory_iterator(parent_path)) {
        if (entry.path().extension() == path.extension()) {
            files.push_back(entry.path().string());
        }
    }
    if (files.size() == 0) {
        return;
    }

    sort(files.begin(), files.end());

    size_t current_file_index = 0;
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i] == m_file_address) {
            current_file_index = i;
            break;
        }
    }

    size_t next_file_index = (current_file_index + direction) % files.size();
    open_file(files[next_file_index]);
};