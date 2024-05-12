#pragma once

#include "../../headers/Common.h"

#include <wx/wx.h>

#include <vector>
#include <functional>
/**
 * @class ThreePointSlider
 * @brief A custom slider control with three thumbs.
 *
 * This class represents a custom slider control with three thumbs that can be used to select values within a range.
 * Each thumb can be moved independently to set its position, and the values between the thumbs can be retrieved.
 * The appearance and behavior of the slider can be customized by setting the colors, thumb radius, and maintaining slide ordering.
 * It also provides a callback function that is triggered when the slider value changes.
 */
class ThreePointSlider : public wxPanel {
    public:
        /**
         * Constructor of ThreePointSlider
         *
         * @param parent Parent window
         * @param id Window ID
         * @param pos Position of the window
         * @param size Size of the window
        */
        ThreePointSlider(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

        /**
         * Set the colors of the thumbs.
         *
         * @param color1 Color of the first thumb
         * @param color2 Color of the second thumb
         * @param color3 Color of the third thumb
         */
        void set_colors(const wxColour &color1, const wxColour &color2, const wxColour &color3);

        /**
         * Get the value of the thumb at the specified index.
         *
         * @param thumb_index Index of the thumb (0, 1, or 2)
         * @return The value of the thumb normalized to the range [0, 1]
         */
        float get_value(int thumb_index) const;

        /**
         * Set the positions of the thumbs.
         *
         * @param positions Vector of positions for the thumbs normalized to the range [0, 1]
         */
        void set_thumbs_positions(const std::vector<float>& positions);

        /**
         * Set whether to maintain slide ordering.
         *
         * @param maintain_ordering True to maintain slide ordering, false otherwise
         */
        void set_maintain_slide_ordering(bool maintain_ordering);

        /**
         * Register a callback function to be called when the slider value changes.
         *
         * @param callback The callback function
         */
        void register_on_change_callback(std::function<void()> callback);

    private:
        float m_thumb_position[3];      // Thumb positions
        int m_thumb_radius = 10;        // Thumb radius
        int m_active_thumb = -1;        // Index of the active thumb (-1 if no thumb is active)
        bool m_maintain_slide_ordering = true;
        wxColour m_thumb_colors[3] = {wxColour(0, 0, 0), wxColour(128, 128, 128), wxColour(255, 255, 255)};
        std::function<void()> m_on_change_callback = nullptr;

        /**
         * Enforce the ordering of the thumbs.
         *
         * @param active_slider_index Index of the active thumb
         */
        void enforce_ordering(int active_slider_index);

        /**
         * Event handler for mouse down event.
         *
         * @param event The mouse event
         */
        void on_mouse_down(wxMouseEvent &event);

        /**
         * Event handler for mouse drag event.
         *
         * @param event The mouse event
         */
        void on_mouse_drag(wxMouseEvent &event);

        /**
         * Event handler for mouse up event.
         *
         * @param event The mouse event
         */
        void on_mouse_up(wxMouseEvent &event);

        /**
         * Event handler for paint event.
         *
         * @param event The paint event
         */
        void on_paint(wxPaintEvent &event);

        /**
         * Convert pixel position to value from interval 0 to 1.
         *
         * @param pixel The pixel position
         * @return The corresponding value
         */
        float pixel_to_value(int pixel) const;

        /**
         * Convert value (from interval 0 to 1) to pixel position.
         *
         * @param value The value
         * @return The corresponding pixel position
         */
        int value_to_pixel(float value) const;
};