#pragma once

#include "../../headers/Common.h"

#include <wx/wx.h>

#include <vector>
#include <functional>
class ThreePointSlider : public wxPanel {
    public:
        ThreePointSlider(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

        void set_colors(const wxColour &color1, const wxColour &color2, const wxColour &color3);

        float get_value(int thumb_index) const;

        void set_thumbs_positions(const std::vector<float>& positions);

        void set_maintain_slide_ordering(bool maintain_ordering);

        void register_on_change_callback(std::function<void()> callback);

    private:
        float m_thumb_position[3];      // Thumb positions
        int m_thumb_radius = 10;        // Thumb radius
        int m_active_thumb = -1;        // Index of the active thumb (-1 if no thumb is active)
        bool m_maintain_slide_ordering = true;
        wxColour m_thumb_colors[3] = {wxColour(0, 0, 0), wxColour(128, 128, 128), wxColour(255, 255, 255)};
        std::function<void()> m_on_change_callback = nullptr;

        void enforce_ordering(int active_slider_index);

        void on_mouse_down(wxMouseEvent &event);

        void on_mouse_drag(wxMouseEvent &event);

        void on_mouse_up(wxMouseEvent &event);

        void on_paint(wxPaintEvent &event);

        float pixel_to_value(int pixel) const;

        int value_to_pixel(float value) const;
};