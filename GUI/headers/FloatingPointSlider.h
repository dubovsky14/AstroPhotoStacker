#pragma once

#include <wx/wx.h>

#include <string>
#include <functional>

class FloatingPointSlider {
    public:
        FloatingPointSlider() = delete;
        FloatingPointSlider(const FloatingPointSlider&) = delete;

        FloatingPointSlider(wxWindow *parent,
                            const std::string &label,
                            float min_value,
                            float max_value,
                            float initial_value,
                            float step,
                            unsigned int n_decimals,
                            std::function<void(float)> callback);

        void set_tooltip(const std::string &tooltip);

        void hide();

        void show();

        void add_sizer(wxSizer *sizer, int proportion, int flag, int border);

    private:
        wxStaticText    *m_text = nullptr;
        wxSlider        *m_slider = nullptr;
};