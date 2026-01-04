#pragma once

#include "../headers/SettingsCustomization.h"
#include "../headers/MainFrame.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <functional>

class SettingsCustomizationGUI : public wxFrame {
    public:
        SettingsCustomizationGUI(MyFrame *parent, const std::function<void()> &on_closure_callback = nullptr);

        ~SettingsCustomizationGUI() override {
            if (m_on_closure_callback) {
                m_on_closure_callback();
            }
        };

    private:
        MyFrame *m_parent = nullptr;
        MetadataViewSettings *m_metadata_view_settings = nullptr;
        FrameStatisticsViewSettings *m_frame_statistics_view_settings = nullptr;

        wxCheckBox *m_show_exposure_time_checkbox = nullptr;
        wxCheckBox *m_show_iso_checkbox = nullptr;
        wxCheckBox *m_show_aperture_checkbox = nullptr;
        wxCheckBox *m_show_temperature_checkbox = nullptr;
        wxCheckBox *m_show_focal_length_checkbox = nullptr;
        wxCheckBox *m_show_resolution_checkbox = nullptr;

        wxCheckBox *m_show_mean_checkbox = nullptr;
        wxCheckBox *m_show_stddev_checkbox = nullptr;
        wxCheckBox *m_show_min_checkbox = nullptr;
        wxCheckBox *m_show_max_checkbox = nullptr;

        void add_checkbox(wxBoxSizer *sizer, const std::string &label, bool *value_pointer, wxCheckBox **checkbox_pointer, const std::string &tooltip = "");

        std::function<void()> m_on_closure_callback = nullptr;
};