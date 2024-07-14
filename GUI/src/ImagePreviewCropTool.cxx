#include "../headers/ImagePreviewCropTool.h"

using namespace std;

ImagePreviewCropTool::ImagePreviewCropTool(wxFrame *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {

    bind_crop_events();

};

void ImagePreviewCropTool::update_preview_bitmap()   {
    update_preview_data(m_image_resize_tool.get_last_mouse_position_relative_x(), m_image_resize_tool.get_last_mouse_position_relative_y());
    plot_crop_rectangle();
    ImagePreview::update_preview_bitmap();
};

void ImagePreviewCropTool::get_crop_coordinates(int *top_left_x, int *top_left_y, int *width, int *height) const {
    *top_left_x = m_crop_top_left_x;
    *top_left_y = m_crop_top_left_y;
    *width = m_crop_width;
    *height = m_crop_height;
};

void ImagePreviewCropTool::plot_crop_rectangle()    {
    if (!image_loaded() || m_crop_width == -1 || m_crop_height == -1) {
        return;
    }

    const int line_width = 2;

    // top line
    plot_full_rectangle(m_crop_top_left_x, m_crop_top_left_y, m_crop_top_left_x + m_crop_width, m_crop_top_left_y+line_width);

    // bottom line
    plot_full_rectangle(m_crop_top_left_x, m_crop_top_left_y+ m_crop_height, m_crop_top_left_x + m_crop_width, m_crop_top_left_y + m_crop_height);

    // left line
    plot_full_rectangle(m_crop_top_left_x, m_crop_top_left_y, m_crop_top_left_x - line_width, m_crop_top_left_y + m_crop_height);

    // right line
    plot_full_rectangle(m_crop_top_left_x + m_crop_width, m_crop_top_left_y, m_crop_top_left_x + m_crop_width + line_width, m_crop_top_left_y + m_crop_height);
};

void ImagePreviewCropTool::plot_full_rectangle(int x1, int y1, int x2, int y2)    {
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

            m_preview_data[0][y*m_width + x] = m_max_value;
            m_preview_data[1][y*m_width + x] = 0;
            m_preview_data[2][y*m_width + x] = 0;
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

    const wxPoint magic_point = wxPoint(0.368*m_width, 0.155*m_height);   // shift the position to the center of the image - wxStaticBitmap is buggy ...

    m_preview_bitmap->Bind(wxEVT_LEFT_DOWN, [this, magic_point](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }


        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();
        screen_pos += magic_point;   // shift the position to the center of the image - wxStaticBitmap is buggy ...

        // Convert the mouse position to client coordinates relative to the wxStaticBitmap
        wxPoint client_position = m_preview_bitmap->ScreenToClient(screen_pos);

        // Check if the mouse is over the wxStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(client_position)) {
            // Calculate the relative position of the mouse within the wxStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(client_position.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(client_position.y) / bitmapSize.GetHeight();

            m_crop_top_left_x = m_image_resize_tool.get_original_coordinate_x(relative_x);
            m_crop_top_left_y = m_image_resize_tool.get_original_coordinate_y(relative_y);
        }
    });

    m_preview_bitmap->Bind(wxEVT_LEFT_UP, [this, magic_point](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }
        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();
        screen_pos += magic_point;   // shift the position to the center of the image - wxStaticBitmap is buggy ...

        // Convert the mouse position to client coordinates relative to the wxStaticBitmap
        wxPoint client_position = m_preview_bitmap->ScreenToClient(screen_pos);

        // Check if the mouse is over the wxStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(client_position)) {
            // Calculate the relative position of the mouse within the wxStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(client_position.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(client_position.y) / bitmapSize.GetHeight();

            m_crop_width = m_image_resize_tool.get_original_coordinate_x(relative_x) - m_crop_top_left_x;
            m_crop_height = m_image_resize_tool.get_original_coordinate_y(relative_y) - m_crop_top_left_y;
        }

        update_preview_bitmap();
    });
};