#include "../headers/HistogramDataToolGUI.h"

#include <algorithm>
#include <iostream>

using namespace std;

HistogramDataToolGUI::HistogramDataToolGUI(wxWindow *parent, wxSizer *sizer, const wxPoint &pos, const wxSize &size) {
    m_parent = parent;
    m_sizer = sizer;
    m_width_pixels = size.GetWidth();
    m_height_pixels = size.GetHeight();

    // Create a wxImage
    wxImage image(m_width_pixels, m_height_pixels);

    // Set the image to black
    for (int x = 0; x < m_width_pixels; ++x) {
        for (int y = 0; y < m_height_pixels; ++y) {
            image.SetRGB(x, y, 0,0,0);
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Create a wxStaticBitmap to display the image
    m_bitmap = new wxStaticBitmap(parent, wxID_ANY, bitmap);

    // Add the wxStaticBitmap to a sizer
    m_sizer->Add(m_bitmap, 1, wxCENTER, 0);
};

void HistogramDataToolGUI::set_background_color(const wxColour &color)  {
    m_background_color = color;
    update_plot();
};

void HistogramDataToolGUI::set_line_colors(const std::vector<wxColour> &colors) {
    m_line_colors = colors;
    update_plot();
};

void HistogramDataToolGUI::set_luminance_color(const wxColour &color)   {
    m_luminance_color = color;
    update_plot();
};

void HistogramDataToolGUI::set_histogram_data_colors(const HistogramDataTool &histogram_data_tool)  {
    m_histogram_data_tool = std::make_unique<HistogramDataTool>(histogram_data_tool);
    update_plot();
};

void HistogramDataToolGUI::set_color_stretcher(const CombinedColorStrecherTool &color_stretching)   {
    m_color_stretcher = std::make_unique<CombinedColorStrecherTool>(color_stretching);
    update_plot();
};

std::vector<float> HistogramDataToolGUI::get_mean_values(bool apply_green_correction) const  {
    if (m_histogram_data_tool != nullptr) {
        return m_histogram_data_tool->get_mean_values(m_color_stretcher == nullptr ? nullptr : m_color_stretcher.get(), apply_green_correction);
    }
    return {};
};

std::vector<float> HistogramDataToolGUI::get_median_values(bool apply_green_correction) const  {
    if (m_histogram_data_tool != nullptr) {
        return m_histogram_data_tool->get_median_values(m_color_stretcher == nullptr ? nullptr : m_color_stretcher.get(), apply_green_correction);
    }
    return {};
};

wxImage HistogramDataToolGUI::get_background()  {
    wxImage image(m_width_pixels, m_height_pixels);
    for (int x = 0; x < m_width_pixels; ++x) {
        for (int y = 0; y < m_height_pixels; ++y) {
            image.SetRGB(x, y, m_background_color.Red(), m_background_color.Green(), m_background_color.Blue());
        }
    }
    return image;
};

void HistogramDataToolGUI::update_plot()    {
    wxImage image = get_background();

    if (m_histogram_data_tool != nullptr) {
        // Get the stretched color data
        const std::vector<std::vector<int>> stretched_color_data = m_color_stretcher != nullptr ?
            m_histogram_data_tool->get_stretched_color_data(*m_color_stretcher) : m_histogram_data_tool->get_histogram_data_colors();

        // Rebin the to fit to histogram width
        vector<vector<int>> rebinned_data;
        for (int i_color = 0; i_color < m_histogram_data_tool->get_number_of_colors(); i_color++) {
            const std::vector<int> &histogram_data = stretched_color_data[i_color];
            const std::vector<int> rebinned_histogram_data = m_histogram_data_tool->rebin_data(histogram_data, m_width_pixels);
            rebinned_data.push_back(rebinned_histogram_data);
        }

        // Scale the histogram data to fit the height of the image
        for (int i_color = 0; i_color < m_histogram_data_tool->get_number_of_colors(); i_color++) {
            const std::vector<int> &histogram_data = rebinned_data[i_color];
            const int edge_to_cut_off = histogram_data.size() > 2 ? 1 : 0;
            const int max_value = *std::max_element(histogram_data.begin()+edge_to_cut_off, histogram_data.end()-edge_to_cut_off);
            for (int pixel_coordinate_x = 0; pixel_coordinate_x < m_width_pixels; pixel_coordinate_x++) {
                const float value = histogram_data.at(pixel_coordinate_x);
                const int height = m_height_pixels*value / max_value;

                //image.SetRGB(pixel_coordinate_x, height, m_line_colors[i_color].Red(), m_line_colors[i_color].Green(), m_line_colors[i_color].Blue());
                for (int pixel_coordinate_y = 0; pixel_coordinate_y < height; pixel_coordinate_y++) {
                    image.SetRGB(pixel_coordinate_x, m_height_pixels - pixel_coordinate_y, m_line_colors[i_color].Red(), m_line_colors[i_color].Green(), m_line_colors[i_color].Blue());
                }

            }
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Set the bitmap to the wxStaticBitmap
    m_bitmap->SetBitmap(bitmap);
};