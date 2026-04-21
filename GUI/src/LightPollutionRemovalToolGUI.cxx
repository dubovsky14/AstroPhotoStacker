#include "../headers/LightPollutionRemovalToolGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"

using namespace AstroPhotoStacker;
using namespace std;

LightPollutionRemovalToolGUI::LightPollutionRemovalToolGUI(MyFrame *parent, const std::vector<std::vector<double>> &stacked_image, int width, int height, PostProcessingTool *post_processing_tool) :
        wxFrame(parent, wxID_ANY, "Light Pollution Removal Tool", wxDefaultPosition, wxSize(1920, 1080))  {
    m_parent = parent;
    m_stacked_image = &stacked_image;
    m_width = width;
    m_height = height;
    m_post_processing_tool = post_processing_tool;

    SetTitle("Light Pollution Removal Tool");
    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_main_vertical_sizer);


    int preview_width = 1400;
    int preview_height = preview_width * float(m_height) / m_width;
    m_image_preview = make_unique<ImagePreviewGridSelector>(this, preview_width, preview_height, 255, false);
    m_image_preview->read_preview_from_stacked_image(*m_stacked_image, m_width, m_height);
    m_image_preview->set_max_zoom_factor(32);
    m_main_vertical_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);
    set_sample_windows(400, 30);
    m_image_preview->update_preview_bitmap();
    add_exposure_correction_spin_ctrl();
};


void LightPollutionRemovalToolGUI::add_exposure_correction_spin_ctrl()   {
    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());
    m_image_preview->set_stretcher(&m_color_stretcher);

    m_exposure_correction_slider = make_unique<FloatingPointSlider>(
        this,
        "Exposure correction: ",
        -7.0,
        7.0,
        0.0,
        0.1,
        1,
        [this](float value){
            IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,value,1);
            m_image_preview->update_preview_bitmap();
        }
    );
    m_exposure_correction_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);
};


void LightPollutionRemovalToolGUI::set_sample_windows(int window_size, int space_between_windows) {
    vector<SampleWindow> sample_windows;
    for (int y = 0; y < m_height; y += window_size + space_between_windows) {
        for (int x = 0; x < m_width; x += window_size + space_between_windows) {
            SampleWindow window;
            window.top_left = {x, y};
            window.bottom_right = {min(x + window_size, m_width), min(y + window_size, m_height)};
            sample_windows.push_back(window);
        }
    }
    m_image_preview->set_grid_windows(sample_windows);
};