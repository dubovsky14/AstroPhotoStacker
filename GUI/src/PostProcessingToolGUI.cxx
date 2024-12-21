#include "../headers/PostProcessingToolGUI.h"

using namespace std;

PostProcessingToolGUI::PostProcessingToolGUI(MyFrame *parent, const std::vector<std::vector<double>> &stacked_image, int width, int height, PostProcessingTool *post_processing_tool) :
        wxFrame(parent, wxID_ANY, "Post processing tool", wxDefaultPosition, wxSize(800, 800))  {

    m_parent = parent;
    m_stacked_image = &stacked_image;
    m_post_processing_tool = post_processing_tool;
    m_width = width;
    m_height = height;

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_main_vertical_sizer);

    int preview_width = 600;
    int preview_height = preview_width * float(height) / width;
    m_image_preview = make_unique<ImagePreview>(this, preview_width, preview_height, 255, true);
    m_image_preview->read_preview_from_stacked_image(stacked_image, width, height);
    m_image_preview->update_preview_bitmap();

    m_main_vertical_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);

    add_rgb_alignment_settings();

    add_sharpening_settings();

    add_apply_button();
};

void PostProcessingToolGUI::add_rgb_alignment_settings()    {
    wxCheckBox* use_rgb_alignment_checkbox = new wxCheckBox(this, wxID_ANY, "Apply rgb alignment");
    use_rgb_alignment_checkbox->SetValue(m_post_processing_tool->get_apply_rgb_alignment());
    use_rgb_alignment_checkbox->Bind(wxEVT_CHECKBOX, [use_rgb_alignment_checkbox, this](wxCommandEvent&){
        const bool is_checked = use_rgb_alignment_checkbox->GetValue();
        m_post_processing_tool->set_apply_rgb_alignment(is_checked);
    });
    m_main_vertical_sizer->Add(use_rgb_alignment_checkbox, 0, wxEXPAND, 5);

    // Red shift
    wxBoxSizer *red_shift_horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);

    FloatingPointSlider* red_shift_x_slider = new FloatingPointSlider(
        this, "Red shift x-coordinate: ", -10, 10, m_post_processing_tool->get_shift_red().first, 1, 0, [this](float shift_x){
            std::pair<int,int> shift_red = m_post_processing_tool->get_shift_red();
            shift_red.first = shift_x;
            m_post_processing_tool->set_shift_red(shift_red);
        }
    );
    red_shift_x_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    FloatingPointSlider* red_shift_y_slider = new FloatingPointSlider(
        this, "Red shift y-coordinate: ", -10, 10, m_post_processing_tool->get_shift_red().second, 1, 0, [this](float shift_y){
            std::pair<int,int> shift_red = m_post_processing_tool->get_shift_red();
            shift_red.second = shift_y;
            m_post_processing_tool->set_shift_red(shift_red);
        }
    );
    red_shift_y_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    m_main_vertical_sizer->Add(red_shift_horizontal_sizer, 0, wxEXPAND, 5);

    // Blue shift
    wxBoxSizer *blue_shift_horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);

    FloatingPointSlider* blue_shift_x_slider = new FloatingPointSlider(
        this, "Blue shift x-coordinate: ", -10, 10, m_post_processing_tool->get_shift_blue().first, 1, 0, [this](float shift_x){
            std::pair<int,int> shift_blue = m_post_processing_tool->get_shift_blue();
            shift_blue.first = shift_x;
            m_post_processing_tool->set_shift_blue(shift_blue);
        }
    );
    blue_shift_x_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    FloatingPointSlider* blue_shift_y_slider = new FloatingPointSlider(
        this, "Blue shift y-coordinate: ", -10, 10, m_post_processing_tool->get_shift_blue().second, 1, 0, [this](float shift_y){
            std::pair<int,int> shift_blue = m_post_processing_tool->get_shift_blue();
            shift_blue.second = shift_y;
            m_post_processing_tool->set_shift_blue(shift_blue);
        }
    );
    blue_shift_y_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    m_main_vertical_sizer->Add(blue_shift_horizontal_sizer, 0, wxEXPAND, 5);

};

void PostProcessingToolGUI::add_sharpening_settings()   {
    wxCheckBox* use_sharpening_checkbox = new wxCheckBox(this, wxID_ANY, "Apply sharpening");
    use_sharpening_checkbox->SetValue(m_post_processing_tool->get_apply_sharpening());
    use_sharpening_checkbox->Bind(wxEVT_CHECKBOX, [use_sharpening_checkbox, this](wxCommandEvent&){
        const bool is_checked = use_sharpening_checkbox->GetValue();
        m_post_processing_tool->set_apply_sharpening(is_checked);
    });
    m_main_vertical_sizer->Add(use_sharpening_checkbox, 0, wxEXPAND, 5);

    const int default_kernel_size = m_post_processing_tool->get_kernel_size();
    m_kernel_size_slider = make_unique<FloatingPointSlider>(
        this, "Kernel size: ", 1, 31, default_kernel_size, 2, 0, [this](float kernel_size){
            int kernel_size_int = int(kernel_size);
            if (kernel_size_int % 2 == 0) {
                kernel_size_int += 1;
            }
            m_post_processing_tool->set_kernel_size(kernel_size_int);
        }
    );
    m_kernel_size_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    const float default_gauss_width = m_post_processing_tool->get_gauss_width();
    m_gauss_width_slider = make_unique<FloatingPointSlider>(
        this, "Gauss width: ", 0, 10, default_gauss_width, 0.1, 1, [this](float gauss_width){
            m_post_processing_tool->set_gauss_width(gauss_width);
        }
    );
    m_gauss_width_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);

    const float default_center_value = m_post_processing_tool->get_center_value();
    m_center_value_slider = make_unique<FloatingPointSlider>(
        this, "Center value: ", 0, 1, default_center_value, 0.01, 2, [this](float center_value){
            m_post_processing_tool->set_center_value(center_value);
        }
    );
    m_center_value_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND, 5);
};

void PostProcessingToolGUI::add_apply_button() {
    wxButton *button_stack_files    = new wxButton(this, wxID_ANY, "Apply post processing", wxDefaultPosition, wxSize(200, 50));
    button_stack_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const vector<vector<double>> &processed_image = m_post_processing_tool->post_process_image(*m_stacked_image, m_width, m_height);
        m_image_preview->read_preview_from_stacked_image(processed_image, m_width, m_height);
        m_image_preview->update_preview_bitmap();
    });
    m_main_vertical_sizer->Add(button_stack_files, 0, wxEXPAND, 5);
};