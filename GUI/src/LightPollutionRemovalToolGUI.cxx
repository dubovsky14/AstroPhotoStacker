#include "../headers/LightPollutionRemovalToolGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/IndividualColorStretchingBlackMidtoneWhite.h"

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
    m_main_horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(m_main_horizontal_sizer);


    m_preview_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_horizontal_sizer->Add(m_preview_sizer, 1, wxLEFT, 0);

    const int preview_height = 900;
    const int preview_width = preview_height * float(m_width) / m_height;
    m_image_preview = make_unique<ImagePreviewGridSelector>(this, preview_width, preview_height, 255, false);
    m_image_preview->read_preview_from_stacked_image(*m_stacked_image, m_width, m_height);
    m_image_preview->set_max_zoom_factor(32);
    m_preview_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxEXPAND, 0);
    generate_sample_windows();
    m_image_preview->update_preview_bitmap();


    m_grid_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_horizontal_sizer->Add(m_grid_settings_sizer, 0, wxEXPAND, 5);
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
    m_exposure_correction_slider->add_sizer(m_preview_sizer, 0, wxEXPAND, 5);

    m_luminance_stretching_slider = new ThreePointSlider(this, wxID_ANY, wxDefaultPosition, wxSize(300, 50));
    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackMidtoneWhite>());
    m_preview_sizer->Add(m_luminance_stretching_slider, 0, wxEXPAND, 5);
    m_luminance_stretching_slider->set_thumbs_positions(vector<float>({0., 0.5, 1.}));
    m_luminance_stretching_slider->register_on_change_callback([this](){
        const float thumb1 = m_luminance_stretching_slider->get_value(0);
        const float thumb2 = m_luminance_stretching_slider->get_value(1);
        const float thumb3 = m_luminance_stretching_slider->get_value(2);
        const float midtone = thumb1 == thumb3 ? 0.5 : thumb2/(thumb3-thumb1);
        IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(1);
        (dynamic_cast<IndividualColorStretchingBlackMidtoneWhite&>(luminance_stretcher)).set_stretching_parameters(thumb1, midtone, thumb3);
        m_image_preview->update_preview_bitmap();
    });
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
    m_number_of_windows_slider->add_sizer(m_grid_settings_sizer, 0, wxEXPAND, 5);

    m_space_as_fraction_of_window_size_slider = make_unique<FloatingPointSlider>(
        this,
        "Space between sample windows as fraction of window size: ",
        0.01,
        1,
        m_space_as_fraction_of_window_size,
        0.05,
        2,
        [this](float value){
            m_space_as_fraction_of_window_size = value;
        }
    );
    m_space_as_fraction_of_window_size_slider->add_sizer(m_grid_settings_sizer, 0, wxEXPAND, 5);


    wxSizer *sizer_buttons_grid = new wxBoxSizer(wxHORIZONTAL);

    wxButton *generate_sample_windows_button = new wxButton(this, wxID_ANY, "Generate sample windows");
    generate_sample_windows_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        generate_sample_windows();
        m_image_preview->update_preview_bitmap();
    });
    sizer_buttons_grid->Add(generate_sample_windows_button, 0, wxEXPAND, 5);

    m_select_deselect_all_windows_button = new wxButton(this, wxID_ANY, "Deselect all windows");
    m_select_deselect_all_windows_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        if (m_all_windows_selected_status) {
            m_select_deselect_all_windows_button->SetLabel("Select all windows");
            m_image_preview->set_selected_status_for_all_windows(false);
            m_all_windows_selected_status = false;
        }
        else    {
            m_select_deselect_all_windows_button->SetLabel("Deselect all windows");
            m_image_preview->set_selected_status_for_all_windows(true);
            m_all_windows_selected_status = true;
        }
        m_image_preview->update_preview_bitmap();
    });
    sizer_buttons_grid->Add(m_select_deselect_all_windows_button, 1, wxEXPAND, 5);


    m_grid_settings_sizer->Add(sizer_buttons_grid, 0, wxEXPAND, 5);


    // Vertical space
    m_grid_settings_sizer->Add(new wxStaticText(this, wxID_ANY, ""), 0, wxEXPAND, 5);

    wxStaticText *gradient_function_label = new wxStaticText(this, wxID_ANY, "Gradient function type: ");
    m_grid_settings_sizer->Add(gradient_function_label, 0, wxEXPAND, 5);
    std::vector<std::string> gradient_function_types = AstroPhotoStacker::GradientFunctionsFactory::get_supported_function_types();
    wxArrayString gradient_function_choices;
    for (const auto &type : gradient_function_types) {
        gradient_function_choices.Add(type);
    }
    wxChoice *gradient_function_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, gradient_function_choices);
    gradient_function_choice->SetStringSelection(m_gradient_function_type);
    gradient_function_choice->Bind(wxEVT_CHOICE, [this, gradient_function_choice](wxCommandEvent&){
        m_gradient_function_type = gradient_function_choice->GetStringSelection().ToStdString();
    });
    m_grid_settings_sizer->Add(gradient_function_choice, 0, wxEXPAND, 5);


    // Vertical space
    m_grid_settings_sizer->Add(new wxStaticText(this, wxID_ANY, ""), 0, wxEXPAND, 5);
    wxSizer *sizer_buttons_gradient = new wxBoxSizer(wxHORIZONTAL);

    wxButton *fit_gradient_button = new wxButton(this, wxID_ANY, "Fit gradient");
    fit_gradient_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const std::vector<SampleWindow> sample_windows = m_image_preview->get_selected_grid_windows();
        const int number_of_selected_windows = sample_windows.size();
        const int number_of_parameters = GradientFunctionsFactory::get_gradient_function(m_gradient_function_type, m_width, m_height)->get_parameters().size();

        if (number_of_selected_windows < number_of_parameters) {
            wxMessageBox("Not enough sample windows selected to fit the gradient. Select at least " + std::to_string(number_of_parameters) + " sample windows.", "Error", wxICON_ERROR);
            return;
        }

        std::vector<std::unique_ptr<LightPollutionGradientBase>> gradient_functions = fit_gradient(*m_stacked_image, m_width, m_height, sample_windows, m_gradient_function_type);
        m_gradient_functions = std::move(gradient_functions);
        m_stacked_image_after_gradient_removal.clear();
        for (unsigned int i_channel = 0; i_channel < m_stacked_image->size(); i_channel++) {
            m_stacked_image_after_gradient_removal.push_back(subtract_gradient((*m_stacked_image)[i_channel], m_width, m_height, *m_gradient_functions[i_channel], true));
        }
        m_image_preview->read_preview_from_stacked_image(m_stacked_image_after_gradient_removal, m_width, m_height);
        m_showing_original_image = false;
        m_image_preview->update_preview_bitmap();

        m_post_processing_tool->set_light_pollution_gradient(m_gradient_functions);
        m_post_processing_tool->set_use_light_pollution_removal(true);
        set_gradient_removal_status(true);
    });
    sizer_buttons_gradient->Add(fit_gradient_button, 1, wxEXPAND, 5);


    m_show_original_image_button = new wxButton(this, wxID_ANY, "Show original image");
    m_show_original_image_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        if (m_gradient_functions.empty()) {
            return;
        }
        if (!m_showing_original_image) {
            m_image_preview->read_preview_from_stacked_image(*m_stacked_image, m_width, m_height);
            m_image_preview->update_preview_bitmap();
            m_show_original_image_button->SetLabel("Show gradient removed image");
            m_showing_original_image = true;
        }
        else    {
            m_image_preview->read_preview_from_stacked_image(m_stacked_image_after_gradient_removal, m_width, m_height);
            m_image_preview->update_preview_bitmap();
            m_show_original_image_button->SetLabel("Show original image");
            m_showing_original_image = false;
        }
    });

    sizer_buttons_gradient->Add(m_show_original_image_button, 1, wxEXPAND, 5);
    m_grid_settings_sizer->Add(sizer_buttons_gradient, 0, wxEXPAND, 5);


    // Vertical space
    m_grid_settings_sizer->Add(new wxStaticText(this, wxID_ANY, ""), 0, wxEXPAND, 5);

    wxSizer *sizer_removal_allowed = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *removal_allowed_label = new wxStaticText(this, wxID_ANY, "Light pollution removal: ");
    m_removal_enabled_disabled_label = new wxStaticText(this, wxID_ANY, "Disabled");
    m_removal_enabled_disabled_label->SetForegroundColour(*wxRED);
    m_toggle_removal_button = new wxButton(this, wxID_ANY, "Enable removal");
    m_toggle_removal_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        if (m_post_processing_tool->get_use_light_pollution_removal()) {
            m_post_processing_tool->set_use_light_pollution_removal(false);
            set_gradient_removal_status(false);
        }
        else    {
            m_post_processing_tool->set_use_light_pollution_removal(true);
            set_gradient_removal_status(true);
        }
    });
    sizer_removal_allowed->Add(removal_allowed_label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    sizer_removal_allowed->Add(m_removal_enabled_disabled_label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    sizer_removal_allowed->Add(m_toggle_removal_button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_grid_settings_sizer->Add(sizer_removal_allowed, 0, wxEXPAND, 5);
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

void LightPollutionRemovalToolGUI::set_gradient_removal_status(bool enabled) {
    if (enabled) {
        m_removal_enabled_disabled_label->SetLabel("Enabled");
        m_removal_enabled_disabled_label->SetForegroundColour(*wxGREEN);
        m_toggle_removal_button->SetLabel("Disable removal");
    } else {
        m_removal_enabled_disabled_label->SetLabel("Disabled");
        m_removal_enabled_disabled_label->SetForegroundColour(*wxRED);
        m_toggle_removal_button->SetLabel("Enable removal");
    }
}