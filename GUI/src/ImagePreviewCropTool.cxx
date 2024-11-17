#include "../headers/ImagePreviewCropTool.h"

using namespace std;

ImagePreviewCropTool::ImagePreviewCropTool(wxFrame *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {
    bind_crop_events();
};

void ImagePreviewCropTool::update_preview_bitmap(bool apply_green_correction)   {
    m_current_preview_is_raw_file = apply_green_correction;
    wxImage image_wx = get_updated_wximage(apply_green_correction);

    plot_crop_rectangle(&image_wx);

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};

void ImagePreviewCropTool::get_crop_coordinates(int *top_left_x, int *top_left_y, int *width, int *height) const {
    *top_left_x = m_crop_top_left_x;
    *top_left_y = m_crop_top_left_y;
    *width = m_crop_width;
    *height = m_crop_height;
};

void ImagePreviewCropTool::plot_crop_rectangle(wxImage *wx_image)   const   {
    if (!image_loaded() || m_crop_width == -1 || m_crop_height == -1) {
        return;
    }

    const int line_width = 2;

    // top line
    plot_full_rectangle(wx_image, m_crop_top_left_x, m_crop_top_left_y, m_crop_top_left_x + m_crop_width, m_crop_top_left_y+line_width);

    // bottom line
    plot_full_rectangle(wx_image, m_crop_top_left_x, m_crop_top_left_y+ m_crop_height, m_crop_top_left_x + m_crop_width, m_crop_top_left_y + m_crop_height);

    // left line
    plot_full_rectangle(wx_image, m_crop_top_left_x, m_crop_top_left_y, m_crop_top_left_x - line_width, m_crop_top_left_y + m_crop_height);

    // right line
    plot_full_rectangle(wx_image, m_crop_top_left_x + m_crop_width, m_crop_top_left_y, m_crop_top_left_x + m_crop_width + line_width, m_crop_top_left_y + m_crop_height);
};

void ImagePreviewCropTool::plot_full_rectangle(wxImage *wx_image, int x1, int y1, int x2, int y2)   const    {
    if (m_crop_width == -1 || m_crop_height == -1) {
        return;
    }

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

            wx_image->SetRGB(x, y, 255, 0, 0);
        }
    }
};

void ImagePreviewCropTool::drop_crop()    {
    m_crop_top_left_x = 0;
    m_crop_top_left_y = 0;
    m_crop_width = -1;
    m_crop_height = -1;

};

void ImagePreviewCropTool::bind_crop_events() {
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

            m_crop_top_left_x = m_image_resize_tool.get_original_coordinate_x(relative_x);
            m_crop_top_left_y = m_image_resize_tool.get_original_coordinate_y(relative_y);
        }
    });

    m_preview_bitmap->Bind(wxEVT_RIGHT_UP, [this](wxMouseEvent &event) {
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

            m_crop_width = m_image_resize_tool.get_original_coordinate_x(relative_x) - m_crop_top_left_x;
            m_crop_height = m_image_resize_tool.get_original_coordinate_y(relative_y) - m_crop_top_left_y;

            if (m_crop_width < 0) {
                m_crop_top_left_x += m_crop_width;
                m_crop_width = -m_crop_width;
            }

            if (m_crop_height < 0) {
                m_crop_top_left_y += m_crop_height;
                m_crop_height = -m_crop_height;
            }
        }

        update_preview_bitmap();
    });
};