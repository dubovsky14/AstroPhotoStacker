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


    const int preview_height = 900;
    const int preview_width = preview_height * float(m_width) / m_height;
    m_image_preview = make_unique<ImagePreviewGridSelector>(this, preview_width, preview_height, 255, false);
    m_image_preview->read_preview_from_stacked_image(*m_stacked_image, m_width, m_height);
    m_image_preview->set_max_zoom_factor(32);
    m_main_vertical_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);
    generate_sample_windows();
    m_image_preview->update_preview_bitmap();
    add_exposure_correction_spin_ctrl();
    add_grid_generation_settings();
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

void LightPollutionRemovalToolGUI::add_grid_generation_settings() {
    m_number_of_windows_slider = make_unique<FloatingPointSlider>(
        this,
        "Number of sample windows per row: ",
        1,
        50,
        m_n_samples_per_row,
        1,
        0,
        [this](float value){
            m_n_samples_per_row = static_cast<int>(value);
        }
    );
    m_number_of_windows_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    wxSizer *sizer_buttons = new wxBoxSizer(wxHORIZONTAL);

    wxButton *generate_sample_windows_button = new wxButton(this, wxID_ANY, "Generate sample windows");
    generate_sample_windows_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        generate_sample_windows();
        m_image_preview->update_preview_bitmap();
    });
    sizer_buttons->Add(generate_sample_windows_button, 0, wxEXPAND, 5);

    wxButton *fit_gradient_button = new wxButton(this, wxID_ANY, "Fit gradient");
    fit_gradient_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // TODO
    });
    sizer_buttons->Add(fit_gradient_button, 0, wxEXPAND, 5);


    wxButton *show_original_image_button = new wxButton(this, wxID_ANY, "Show original image");
    show_original_image_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        m_image_preview->read_preview_from_stacked_image(*m_stacked_image, m_width, m_height);
        m_image_preview->update_preview_bitmap();
    });
    sizer_buttons->Add(show_original_image_button, 0, wxEXPAND, 5);


    m_main_vertical_sizer->Add(sizer_buttons, 0, wxEXPAND, 5);

};

void LightPollutionRemovalToolGUI::generate_sample_windows() {
    vector<SampleWindow> sample_windows;
    const int window_size = m_width / (m_n_samples_per_row * (1 + m_space_as_fraction_of_window_size) + m_space_as_fraction_of_window_size);
    const int space_between_windows = window_size * m_space_as_fraction_of_window_size;

    const int step_size_x = window_size + space_between_windows;

    const int n_windows_per_column = (m_height - space_between_windows) / step_size_x;
    const int window_height = (m_height - space_between_windows) / n_windows_per_column - space_between_windows;
    const int step_size_y = window_height + space_between_windows;

    for (int y = space_between_windows; y <= m_height - step_size_y; y += step_size_y) {
        for (int x = space_between_windows; x <= m_width - step_size_x; x += step_size_x) {
            SampleWindow window;
            window.top_left = {x, y};
            window.bottom_right = {min(x + window_size, m_width), min(y + window_height, m_height)};
            sample_windows.push_back(window);
        }
    }
    m_image_preview->set_grid_windows(sample_windows);
};