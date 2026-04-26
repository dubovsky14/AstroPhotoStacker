#pragma once

#include "../../headers/LightPollutionRemovalTool.h"
#include "../../headers/LightPollutionGradientFunctions.h"
#include "../headers/ThreePointSlider.h"

#include "../headers/ImagePreviewGridSelector.h"
#include "../headers/PostProcessingTool.h"
#include "../headers/MainFrame.h"

#include <memory>
#include <vector>
#include <utility>


#include <wx/wx.h>
#include <wx/spinctrl.h>

class LightPollutionRemovalToolGUI : public wxFrame  {
    public:
        LightPollutionRemovalToolGUI() = delete;

        LightPollutionRemovalToolGUI(MyFrame *parent, const std::vector<std::vector<double>> &stacked_image, int width, int height, AstroPhotoStacker::PostProcessingTool *post_processing_tool);

    private:
        void generate_sample_windows();

        void add_exposure_correction_spin_ctrl();

        void add_grid_generation_settings();

        CombinedColorStrecherTool m_color_stretcher; // for exposure correction
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;
        ThreePointSlider *m_luminance_stretching_slider  = nullptr;

        std::unique_ptr<FloatingPointSlider> m_number_of_windows_slider     = nullptr;
        std::unique_ptr<FloatingPointSlider> m_space_as_fraction_of_window_size_slider = nullptr;
        std::string m_gradient_function_type = "polynomial3n";

        MyFrame *m_parent = nullptr;
        const std::vector<std::vector<double>> *m_stacked_image = nullptr;
        int m_width  = 0;
        int m_height = 0;

        std::vector<std::vector<double>> m_stacked_image_after_gradient_removal;

        int m_n_samples_per_row = 10;
        float m_space_as_fraction_of_window_size = 0.05;

        std::vector<std::unique_ptr<AstroPhotoStacker::LightPollutionGradientBase>> m_gradient_functions; // fitted gradient functions for each color channel
        wxBoxSizer *m_main_horizontal_sizer = nullptr;
        wxBoxSizer *m_preview_sizer = nullptr;
        wxBoxSizer *m_grid_settings_sizer = nullptr;

        wxStaticText *m_removal_enabled_disabled_label = nullptr;
        AstroPhotoStacker::PostProcessingTool *m_post_processing_tool = nullptr;

        std::unique_ptr<ImagePreviewGridSelector> m_image_preview = nullptr;

        wxButton *m_show_original_image_button = nullptr;
        bool m_showing_original_image = true;

        wxButton *m_toggle_removal_button = nullptr;
        void set_gradient_removal_status(bool enabled);
};