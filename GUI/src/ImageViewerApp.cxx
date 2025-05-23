#include "../headers/ImageViewerApp.h"
#include "../headers/Common.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"

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
    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());
    m_image_preview->set_stretcher(&m_color_stretcher);

    m_preview_panel_sizer = new wxBoxSizer(wxVERTICAL);
    m_preview_and_metadata_sizer->Add(m_preview_panel_sizer, 7, wxEXPAND | wxALL, 5);
    m_preview_panel_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxALIGN_CENTER_HORIZONTAL, 0);

    add_exposure_correction_spin_ctrl();

    add_metadata_panel();

    add_bottom_panel();

    bind_key_events();

    CreateStatusBar();
    SetSizer(m_sizer_main_frame);
    SetStatusText("Author: Michal Dubovsky");

};

void ImageViewerFrame::add_exposure_correction_spin_ctrl()   {
    // Create a wxStaticText to display the current value
    wxStaticText* exposure_correction_text = new wxStaticText(this, wxID_ANY, "Exposure correction: 0.0");

    // Create the wxSlider
    wxSlider* slider_exposure = new wxSlider(this, wxID_ANY, 0, -70, 70, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_exposure->Bind(wxEVT_SLIDER, [exposure_correction_text, slider_exposure, this](wxCommandEvent&){
        const float exposure_correction = slider_exposure->GetValue()/10.;
        //m_current_preview->set_exposure_correction( exposure_correction );

        IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
        (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,exposure_correction,1);

        m_image_preview->update_preview_bitmap();
        const std::string new_label = "Exposure correction: " + to_string(exposure_correction+0.0001).substr(0,4);
        exposure_correction_text->SetLabel(new_label);
    });

    // Add the controls to a sizer
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_preview_panel_sizer->Add(exposure_correction_text, 0,   wxEXPAND, 5);
    m_preview_panel_sizer->Add(slider_exposure, 0,  wxEXPAND, 5);

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
    m_file_address = file_address;

    const std::filesystem::path path = m_file_address;
    const std::filesystem::path parent_path = path.parent_path();
    const std::string extension = path.extension().string();
    const std::vector<std::string> files = get_files_in_folder_with_extension(parent_path.string(), extension);
    const size_t current_file_index = get_current_file_index(files);

    const std::string text = m_file_address + " (" + to_string(current_file_index+1) + "/" + to_string(files.size()) + ")";

    m_file_address_text->SetLabel(text);

    try {
        const Metadata metadata = read_metadata(InputFrame(m_file_address));
        update_metadata(metadata);
    }
    catch (const std::exception &e) {
        cout << e.what() << endl;
    }

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
        else if (event.GetKeyCode() == WXK_DELETE) {
            std::filesystem::remove(m_file_address);
            load_consecutive_file(1);
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
    const std::filesystem::path path = m_file_address;
    const std::filesystem::path parent_path = path.parent_path();
    m_current_folder = parent_path.string();
    const std::string extension = path.extension().string();
    const std::vector<std::string> files = get_files_in_folder_with_extension(parent_path.string(), extension);

    if (files.size() == 0) {
        return;
    }

    const size_t current_file_index = get_current_file_index(files);
    size_t next_file_index = (current_file_index + direction) % files.size();
    open_file(files[next_file_index]);
};

std::vector<std::string> ImageViewerFrame::get_files_in_folder_with_extension(const std::string &folder, const std::string &extension)  const   {
    const std::filesystem::path parent_path = folder;
    vector<string> files;
    for (const auto &entry : std::filesystem::directory_iterator(parent_path)) {
        if (entry.path().extension() == extension) {
            files.push_back(entry.path().string());
        }
    }
    sort(files.begin(), files.end());
    return files;
};

size_t ImageViewerFrame::get_current_file_index(const std::vector<std::string> &files)   const {
    size_t current_file_index = 0;
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i] >= m_file_address) {
            current_file_index = i;
            break;
        }
    }
    return current_file_index;
};