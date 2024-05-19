#pragma once

#include "../headers/HistogramDataTool.h"
#include "../headers/CombinedColorStrecherTool.h"

#include <wx/wx.h>

#include <vector>
#include <memory>

class HistogramDataToolGUI  {
    public:
        /**
         * Constructor of HistogramDataToolGUI
         *
         * @param parent Parent window
         * @param id Window ID
         * @param pos Position of the window
         * @param size Size of the window
        */
        HistogramDataToolGUI(wxWindow *parent, wxSizer *sizer, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

        /**
         * Set background color
         *
         * @param color
        */
        void set_background_color(const wxColour &color);

        /**
         * Set the colors of the histograms
         *
         * @param colors    Vector of colors for the color histogram colors
        */
        void set_line_colors(const std::vector<wxColour> &colors);

        /**
         * Set color for luminance histogram
         *
         * @param color Color for luminance histogram
        */
        void set_luminance_color(const wxColour &color);

        /**
         * Set the histogram data for the color histogram.
         *
         * @param histogram_data_tool   histogram data for the color histogram
        */
        void set_histogram_data_colors(const HistogramDataTool &histogram_data_tool);

        /**
         * Set color stretcher
         *
         * @param color_stretching Color stretcher
        */
        void set_color_stretcher(const CombinedColorStrecherTool &color_stretching);

        /**
         * @Get mean values of the colors
         *
         * @return Vector of mean values of the colors, return empty vector if histogram data tool is not set
        */
        std::vector<float> get_mean_values(bool apply_green_correction = true) const;

    private:

        wxImage get_background();

        void update_plot();

        std::unique_ptr<HistogramDataTool>          m_histogram_data_tool   = nullptr;
        std::unique_ptr<CombinedColorStrecherTool>  m_color_stretcher       = nullptr;

        int m_height_pixels = 0;
        int m_width_pixels = 0;

        wxWindow *m_parent = nullptr;
        wxSizer *m_sizer = nullptr;
        wxColour m_background_color = wxColour(255, 255, 255);
        std::vector<wxColour> m_line_colors = {wxColour(255, 0, 0), wxColour(0, 255, 0), wxColour(0, 0, 255)};
        wxColour m_luminance_color = wxColour(0, 0, 0);

        wxStaticBitmap *m_bitmap = nullptr;
};