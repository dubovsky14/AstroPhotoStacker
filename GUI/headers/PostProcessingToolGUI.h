#pragma once

#include "../../headers/PostProcessingTool.h"

#include "../headers/MainFrame.h"
#include "../headers/ImagePreview.h"
#include "../headers/FloatingPointSlider.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <memory>

class PostProcessingToolGUI : public wxFrame  {
    public:
        PostProcessingToolGUI(MyFrame *parent, const std::vector<std::vector<double>> &stacked_image, int width, int height, AstroPhotoStacker::PostProcessingTool *post_processing_tool);


    private:
        MyFrame *m_parent = nullptr;
        const std::vector<std::vector<double>> *m_stacked_image = nullptr;
        AstroPhotoStacker::PostProcessingTool *m_post_processing_tool = nullptr;
        int m_width;
        int m_height;

        wxBoxSizer *m_main_vertical_sizer = nullptr;
        std::unique_ptr<ImagePreview> m_image_preview = nullptr;

        CombinedColorStrecherTool m_color_stretcher; // for exposure correction
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;

        void add_image_preview();

        void add_exposure_correction_spin_ctrl();

        void add_rgb_alignment_settings();

        void add_sharpening_settings();

        void add_apply_button();

        std::unique_ptr<FloatingPointSlider> m_kernel_size_slider = nullptr;
        std::unique_ptr<FloatingPointSlider> m_gauss_width_slider = nullptr;
        std::unique_ptr<FloatingPointSlider> m_center_value_slider = nullptr;

};