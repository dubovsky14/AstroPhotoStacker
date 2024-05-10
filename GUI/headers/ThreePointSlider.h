#pragma once

#include "../../headers/Common.h"

#include <wx/wx.h>
#include <vector>

class ThreePointSlider : public wxPanel {
    public:
        ThreePointSlider(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize)
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
        }

        float get_value(int thumb_index) const {
            if (thumb_index < 0 || thumb_index > 2) {
                return 0;
            }
            return m_thumb_position[thumb_index];
        }

        void set_initial_thumbs_positions(const std::vector<float>& positions) {
            if (positions.size() != 3) {
                return;
            }
            m_thumb_position[0] = positions[0];
            m_thumb_position[1] = positions[1];
            m_thumb_position[2] = positions[2];
            Refresh();
            std::cout << "Intial positions set: Thumb 0: " << get_value(0) << ", Thumb 1: " << get_value(1) << ", Thumb 2: " << get_value(2) << std::endl;
        }

    private:
        float m_thumb_position[3];         // Thumb positions
        int m_thumb_radius = 10;        // Thumb radius
        int m_active_thumb = -1;        // Index of the active thumb (-1 if no thumb is active)

        void on_mouse_down(wxMouseEvent &event)   {
            // Check if a thumb was clicked
            for (int i = 0; i < 3; i++)  {
                if (abs(event.GetX() - value_to_pixel(m_thumb_position[i])) < m_thumb_radius) {
                    m_active_thumb = i;
                    break;
                }
            }
        }

        void on_mouse_drag(wxMouseEvent &event)   {
            // If a thumb is active, move it
            if (m_active_thumb != -1)  {
                int new_x = event.GetX();

                new_x = AstroPhotoStacker::force_range(new_x, m_thumb_radius, GetSize().GetWidth() - m_thumb_radius);

                m_thumb_position[m_active_thumb] = pixel_to_value(new_x);
                Refresh(); // Trigger a paint event
            }
        }

        void on_mouse_up(wxMouseEvent &event)     {
            // Deactivate the active thumb
            m_active_thumb = -1;
            std::cout << "Thumb 0: " << get_value(0) << ", Thumb 1: " << get_value(1) << ", Thumb 2: " << get_value(2) << std::endl;
        }

        void on_paint(wxPaintEvent &event) {
            wxPaintDC dc(this);

            // Draw the track
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawLine(0, GetSize().GetHeight() / 2, GetSize().GetWidth(), GetSize().GetHeight() / 2);

            // Draw the thumbs
            wxColour thumb_colors[3] = {wxColour(0, 0, 0), wxColour(128, 128, 128), wxColour(255, 255, 255)};
            wxColour thumb_outline_colors[3] = {wxColour(0, 0, 0), wxColour(0, 0, 0), wxColour(0, 0, 0)};
            for (int i = 0; i < 3; i++) {
                dc.SetBrush(wxBrush(thumb_colors[i]));
                dc.SetPen(wxPen(thumb_outline_colors[i]));
                dc.DrawCircle(value_to_pixel(m_thumb_position[i]), GetSize().GetHeight() / 2, 10); // 10 is the thumb radius
            }
        }

        float pixel_to_value(int pixel) {
            return (float(pixel) - m_thumb_radius) / (GetSize().GetWidth() - 2*m_thumb_radius);
        }

        int value_to_pixel(float value) {
            return value * (GetSize().GetWidth() - 2*m_thumb_radius) + m_thumb_radius;
        }
};