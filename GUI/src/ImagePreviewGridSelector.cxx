#include "../headers/ImagePreviewGridSelector.h"

using namespace std;

ImagePreviewGridSelector::ImagePreviewGridSelector(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {

    bind_window_selection_events();
};

void ImagePreviewGridSelector::update_preview_bitmap()   {
    wxImage image_wx = get_updated_wximage();

    plot_sample_windows(&image_wx);

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};

void ImagePreviewGridSelector::set_grid_windows(const std::vector<AstroPhotoStacker::SampleWindow> &grid_windows) {
    m_grid_windows_coordinates_and_validity.clear();
    for (const auto &window : grid_windows) {
        m_grid_windows_coordinates_and_validity.push_back({window, true});
    }
};

void ImagePreviewGridSelector::set_selected_status_for_all_windows(bool selected) {
    for (auto &window_coordinates_and_validity : m_grid_windows_coordinates_and_validity) {
        window_coordinates_and_validity.second = selected;
    }
};

std::vector<AstroPhotoStacker::SampleWindow> ImagePreviewGridSelector::get_selected_grid_windows() const {
    std::vector<AstroPhotoStacker::SampleWindow> selected_windows;
    for (const auto &window_coordinates_and_validity : m_grid_windows_coordinates_and_validity) {
        if (window_coordinates_and_validity.second) {
            selected_windows.push_back(window_coordinates_and_validity.first);
        }
    }
    return selected_windows;
};

void ImagePreviewGridSelector::plot_sample_windows(wxImage *wx_image)   const   {
    if (!image_loaded() || m_grid_windows_coordinates_and_validity.empty()) {
        return;
    }

    const int line_width = 1;

    for (const auto &window_coordinates_and_validity : m_grid_windows_coordinates_and_validity) {
        const AstroPhotoStacker::SampleWindow &window = window_coordinates_and_validity.first;
        const bool valid = window_coordinates_and_validity.second;

        const std::array<int, 3> color = valid ? std::array<int, 3>{0, 255, 0} : std::array<int, 3>{255, 0, 0};

        // top line
        plot_full_rectangle(wx_image, window.top_left.first, window.top_left.second, window.bottom_right.first, window.top_left.second+line_width, color);

        // bottom line
        plot_full_rectangle(wx_image, window.top_left.first, window.bottom_right.second, window.bottom_right.first, window.bottom_right.second+line_width, color);

        // left line
        plot_full_rectangle(wx_image, window.top_left.first, window.top_left.second, window.top_left.first - line_width, window.bottom_right.second, color);

        // right line
        plot_full_rectangle(wx_image, window.bottom_right.first, window.top_left.second, window.bottom_right.first + line_width, window.bottom_right.second, color);
    }
};

void ImagePreviewGridSelector::plot_full_rectangle(wxImage *wx_image, int x1, int y1, int x2, int y2, const std::array<int, 3> &color)   const    {
    int x1_resized = m_image_resize_tool.get_preview_coordinate_x(x1);
    int y1_resized = m_image_resize_tool.get_preview_coordinate_y(y1);

    int x2_resized = m_image_resize_tool.get_preview_coordinate_x(x2);
    int y2_resized = m_image_resize_tool.get_preview_coordinate_y(y2);

    if (x1_resized > x2_resized) {
        swap(x1_resized, x2_resized);
    }

    if (y1_resized > y2_resized) {
        swap(y1_resized, y2_resized);
    }

    for (int x = x1_resized; x <= x2_resized; ++x) {
        if (x < 0 || x >= m_width) {
            continue;
        }
        for (int y = y1_resized; y <= y2_resized; ++y) {
            if (y < 0 || y >= m_height) {
                continue;
            }

            wx_image->SetRGB(x, y, color[0], color[1], color[2]);
        }
    }
};

void ImagePreviewGridSelector::bind_window_selection_events() {
    m_preview_bitmap->Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }

        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();

        // Check if the mouse is over the wxStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(screen_pos)) {
            // Calculate the relative position of the mouse within the wxStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(screen_pos.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(screen_pos.y) / bitmapSize.GetHeight();

            const int mouse_x_original = m_image_resize_tool.get_original_coordinate_x(relative_x);
            const int mouse_y_original = m_image_resize_tool.get_original_coordinate_y(relative_y);

            for (auto &window_coordinates_and_validity : m_grid_windows_coordinates_and_validity) {
                AstroPhotoStacker::SampleWindow &window = window_coordinates_and_validity.first;
                bool &valid = window_coordinates_and_validity.second;

                if (mouse_x_original >= window.top_left.first && mouse_x_original <= window.bottom_right.first &&
                    mouse_y_original >= window.top_left.second && mouse_y_original <= window.bottom_right.second) {
                    valid = !valid;
                    break;
                }
            }
        }
        update_preview_bitmap();
    });

};