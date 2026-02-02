#include "../headers/SettingsCustomizationGUI.h"

using namespace std;


SettingsCustomizationGUI::SettingsCustomizationGUI(MyFrame *parent, const std::function<void()> &on_closure_callback)
    : wxFrame(parent, wxID_ANY, "Produce aligned images", wxDefaultPosition, wxSize(1000, 1000)),
        m_parent(parent),
        m_on_closure_callback(on_closure_callback) {
    SettingsCustomization &settings_customization = SettingsCustomization::get_instance();
    m_metadata_view_settings = &settings_customization.metadata_view_settings;
    m_frame_statistics_view_settings = &settings_customization.frame_statistics_view_settings;


    this->SetTitle("Settings Customization");
    this->SetSize(wxSize(400, 600));
    this->SetMinSize(wxSize(400, 400));


    wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *metadata_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *metadata_label = new wxStaticText(this, wxID_ANY, "Metadata View Settings:");
    metadata_sizer->Add(metadata_label, 0, wxALL, 5);

    add_checkbox(metadata_sizer, "Show Exposure Time", &m_metadata_view_settings->show_exposure_time, &m_show_exposure_time_checkbox);
    add_checkbox(metadata_sizer, "Show ISO", &m_metadata_view_settings->show_iso, &m_show_iso_checkbox);
    add_checkbox(metadata_sizer, "Show Aperture", &m_metadata_view_settings->show_aperture, &m_show_aperture_checkbox);
    add_checkbox(metadata_sizer, "Show Temperature", &m_metadata_view_settings->show_temperature, &m_show_temperature_checkbox);
    add_checkbox(metadata_sizer, "Show Focal Length", &m_metadata_view_settings->show_focal_length, &m_show_focal_length_checkbox);
    add_checkbox(metadata_sizer, "Show DateTime", &m_metadata_view_settings->show_datetime, &m_show_datetime_checkbox);
    //add_checkbox(metadata_sizer, "Show Resolution", &m_metadata_view_settings->show_resolution, &m_show_resolution_checkbox);

    main_sizer->Add(metadata_sizer, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer *frame_stats_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *frame_stats_label = new wxStaticText(this, wxID_ANY, "Frame Statistics View Settings:");
    frame_stats_sizer->Add(frame_stats_label, 0, wxALL, 5);

    add_checkbox(frame_stats_sizer, "Show Mean", &m_frame_statistics_view_settings->show_mean, &m_show_mean_checkbox);
    add_checkbox(frame_stats_sizer, "Show Standard Deviation", &m_frame_statistics_view_settings->show_stddev, &m_show_stddev_checkbox);
    add_checkbox(frame_stats_sizer, "Show Minimum", &m_frame_statistics_view_settings->show_min, &m_show_min_checkbox);
    add_checkbox(frame_stats_sizer, "Show Maximum", &m_frame_statistics_view_settings->show_max, &m_show_max_checkbox);

    main_sizer->Add(frame_stats_sizer, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer *other_settings_sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText *other_settings_label = new wxStaticText(this, wxID_ANY, "Other Settings:");
    other_settings_sizer->Add(other_settings_label, 0, wxALL, 5);
    m_other_settings_customization = &settings_customization.other_settings_customization;
    add_checkbox(other_settings_sizer, "Show Full Frame Paths", &m_other_settings_customization->show_full_frame_paths, &m_show_full_frame_paths_checkbox,
        "If checked, full file paths will be shown in the file list. Otherwise, only file names will be shown.");

    main_sizer->Add(other_settings_sizer, 0, wxEXPAND | wxALL, 10);


    this->SetSizer(main_sizer);
    this->Layout();
}

void SettingsCustomizationGUI::add_checkbox(wxBoxSizer *sizer, const std::string &label, bool *value_pointer, wxCheckBox **checkbox_pointer, const std::string &tooltip) {
    *checkbox_pointer = new wxCheckBox(this, wxID_ANY, label);
    (*checkbox_pointer)->SetValue(*value_pointer);
    if (!tooltip.empty()) {
        (*checkbox_pointer)->SetToolTip(tooltip);
    }

    (*checkbox_pointer)->Bind(wxEVT_CHECKBOX, [this, checkbox_pointer, value_pointer](wxCommandEvent&){
        const bool checked = (*checkbox_pointer)->IsChecked();
        *value_pointer = checked;
    });

    sizer->Add(*checkbox_pointer, 0, wxALL, 5);
}
