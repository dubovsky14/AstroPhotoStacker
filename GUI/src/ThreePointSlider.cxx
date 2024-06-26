#include "../headers/ThreePointSlider.h"

ThreePointSlider::ThreePointSlider(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
    : wxPanel(parent, id, pos, size)    {
    // Initialize thumb positions
    m_thumb_position[0] = 0;
    m_thumb_position[1] = 0.5;
    m_thumb_position[2] = 1;

    Bind(wxEVT_PAINT, &ThreePointSlider::on_paint, this);

    // Bind mouse events
    Bind(wxEVT_LEFT_DOWN, &ThreePointSlider::on_mouse_down, this);
    Bind(wxEVT_MOTION, &ThreePointSlider::on_mouse_drag, this);
    Bind(wxEVT_LEFT_UP, &ThreePointSlider::on_mouse_up, this);
};

void ThreePointSlider::set_colors(const wxColour &color1, const wxColour &color2, const wxColour &color3) {
    m_thumb_colors[0] = color1;
    m_thumb_colors[1] = color2;
    m_thumb_colors[2] = color3;
    Refresh();
};

float ThreePointSlider::get_value(int thumb_index) const {
    if (thumb_index < 0 || thumb_index > 2) {
        return 0;
    }
    return m_thumb_position[thumb_index];
};

void ThreePointSlider::set_thumbs_positions(const std::vector<float>& positions) {
    if (positions.size() != 3) {
        return;
    }
    m_thumb_position[0] = positions[0];
    m_thumb_position[1] = positions[1];
    m_thumb_position[2] = positions[2];
    Refresh();
};

void ThreePointSlider::set_maintain_slide_ordering(bool maintain_ordering) {
    m_maintain_slide_ordering = maintain_ordering;
};

void ThreePointSlider::register_on_change_callback(std::function<void()> callback) {
    m_on_change_callback = callback;
};


void ThreePointSlider::enforce_ordering(int active_slider_index)  {
    if (!m_maintain_slide_ordering) {
        return;
    }

    if (active_slider_index == 0) {
        m_thumb_position[0] = AstroPhotoStacker::force_range<float>(m_thumb_position[0], 0, m_thumb_position[1]);
    }
    else if (active_slider_index == 1) {
        m_thumb_position[1] = AstroPhotoStacker::force_range<float>(m_thumb_position[1], m_thumb_position[0], m_thumb_position[2]);
    }
    else if (active_slider_index == 2) {
        m_thumb_position[2] = AstroPhotoStacker::force_range<float>(m_thumb_position[2], m_thumb_position[1], 1);
    }
};

void ThreePointSlider::on_mouse_down(wxMouseEvent &event)   {
    // Check if a thumb was clicked
    for (int i = 2; i >= 0; i--)  { // descending order to select the thumb on top, if more than one thumb is at the same position
        if (abs(event.GetX() - value_to_pixel(m_thumb_position[i])) < m_thumb_radius) {
            m_active_thumb = i;
            break;
        }
    }
};

void ThreePointSlider::on_mouse_drag(wxMouseEvent &event)   {
    // If a thumb is active, move it
    if (m_active_thumb != -1)  {
        int new_x = event.GetX();

        new_x = AstroPhotoStacker::force_range(new_x, m_thumb_radius, GetSize().GetWidth() - m_thumb_radius);

        m_thumb_position[m_active_thumb] = pixel_to_value(new_x);
        enforce_ordering(m_active_thumb);
        Refresh(); // Trigger a paint event
    }
};

void ThreePointSlider::on_mouse_up(wxMouseEvent &event)     {
    // Deactivate the active thumb
    m_active_thumb = -1;
    if (m_on_change_callback != nullptr) {
        m_on_change_callback();
    }
};

void ThreePointSlider::on_paint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    // Draw the track
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(0, GetSize().GetHeight() / 2, GetSize().GetWidth(), GetSize().GetHeight() / 2);

    // Draw the thumbs
    wxColour thumb_outline_colors[3] = {wxColour(0, 0, 0), wxColour(0, 0, 0), wxColour(0, 0, 0)};
    for (int i = 0; i < 3; i++) {
        dc.SetBrush(wxBrush(m_thumb_colors[i]));
        dc.SetPen(wxPen(thumb_outline_colors[i]));
        dc.DrawCircle(value_to_pixel(m_thumb_position[i]), GetSize().GetHeight() / 2, 10); // 10 is the thumb radius
    }
};

float ThreePointSlider::pixel_to_value(int pixel)   const {
    return (float(pixel) - m_thumb_radius) / (GetSize().GetWidth() - 2*m_thumb_radius);
};

int ThreePointSlider::value_to_pixel(float value)   const {
    return value * (GetSize().GetWidth() - 2*m_thumb_radius) + m_thumb_radius;
};
