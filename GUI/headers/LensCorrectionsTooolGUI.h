#pragma once

#include "../headers/MainFrame.h"
#include "../headers/ImagePreview.h"
#include "../headers/FloatingPointSlider.h"

#include "../../headers/InputFrame.h"
#include "../../headers/LensCorrectionsTool.h"


#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <memory>
#include <vector>
#include <string>


class LensCorrectionsToolGUI : public wxFrame  {
    public:
        /**
         * @brief Construct a new Alignment Frame object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param aligned_images_producer pointer to the aligned images producer object
         */
        LensCorrectionsToolGUI(MyFrame *parent, const AstroPhotoStacker::InputFrame &reference_frame);


    private:
        MyFrame *m_parent = nullptr;
        wxBoxSizer *m_main_vertical_sizer = nullptr;
        wxBoxSizer *m_basic_settings_sizer = nullptr;
        wxBoxSizer *m_image_preview_sizer = nullptr;

        std::unique_ptr<ImagePreview> m_image_preview = nullptr;
        std::unique_ptr<AstroPhotoStacker::LensCorrectionsTool> m_lens_corrections_tool = nullptr;
        AstroPhotoStacker::InputFrame m_reference_frame;

        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;

        CombinedColorStrecherTool m_exposure_stretcher; // for exposure correction

        void initialize_gui();

        void add_exposure_correction_spin_ctrl();
};
