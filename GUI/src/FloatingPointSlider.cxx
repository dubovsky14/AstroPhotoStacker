#include "../headers/FloatingPointSlider.h"

#include "../headers/Common.h"

#include <cmath>

FloatingPointSlider::FloatingPointSlider(   wxWindow *parent,
                                            const std::string &label,
                                            float min_value,
                                            float max_value,
                                            float initial_value,
                                            float step,
                                            unsigned int n_decimals,
                                            std::function<void(float)> callback)    {


    const float ten_power_n_decimals = pow(10, n_decimals);
    const wxString label_with_value = label + get_rounded_value(initial_value + (initial_value > 0 ? 0.00001 : -0.00001), n_decimals);
    m_text = new wxStaticText(parent, wxID_ANY, label_with_value);
    m_slider = new wxSlider(parent, wxID_ANY, initial_value*ten_power_n_decimals, min_value*ten_power_n_decimals, max_value*ten_power_n_decimals, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    m_slider->Bind(wxEVT_SLIDER, [this, ten_power_n_decimals, label, n_decimals, callback](wxCommandEvent&){
        const float value = m_slider->GetValue()/ten_power_n_decimals;
        const std::string new_label = label + get_rounded_value(value + (value > 0 ? 0.00001 : -0.00001), n_decimals);
        m_text->SetLabel(new_label);
        callback(value);
    });
};

void FloatingPointSlider::set_tool_tip(const std::string &tooltip)   {
    m_slider->SetToolTip(tooltip);
};

void FloatingPointSlider::hide()    {
    m_text->Hide();
    m_slider->Hide();
};

void FloatingPointSlider::show()    {
    m_text->Show();
    m_slider->Show();
};

void FloatingPointSlider::add_sizer(wxSizer *sizer, int proportion, int flag, int border)   {
    sizer->Add(m_text, proportion, flag, border);
    sizer->Add(m_slider, proportion, flag, border);
};