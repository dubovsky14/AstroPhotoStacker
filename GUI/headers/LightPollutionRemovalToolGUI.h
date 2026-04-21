#pragma once

#include "../../headers/LightPollutionRemovalTool.h"
#include "../../headers/LightPollutionGradientFunctions.h"

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
        void set_sample_windows(int window_size, int space_between_windows);

        void fit_gradient();

        void add_exposure_correction_spin_ctrl();

        CombinedColorStrecherTool m_color_stretcher; // for exposure correction
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;

        MyFrame *m_parent = nullptr;
        const std::vector<std::vector<double>> *m_stacked_image = nullptr;
        int m_width  = 0;
        int m_height = 0;

        std::vector<std::unique_ptr<AstroPhotoStacker::LightPollutionGradientBase>> m_gradient_functions; // fitted gradient functions for each color channel
        wxBoxSizer *m_main_vertical_sizer = nullptr;
        AstroPhotoStacker::PostProcessingTool *m_post_processing_tool = nullptr;

        std::unique_ptr<ImagePreviewGridSelector> m_image_preview = nullptr;

};