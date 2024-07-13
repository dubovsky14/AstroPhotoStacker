#include "../headers/ImagePreviewCropTool.h"

using namespace std;

ImagePreviewCropTool::ImagePreviewCropTool(wxFrame *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {

};

void ImagePreviewCropTool::plot_crop_rectangle()    {
    if (m_crop_width == -1 || m_crop_height == -1) {
        return;
    }

    const int line_width = 2;

    // top line
    plot_full_rectangle(m_crop_top_left_x, m_crop_top_left_y, m_crop_top_left_x + m_crop_width, m_crop_top_left_y+line_width);

    // bottom line
    plot_full_rectangle(m_crop_top_left_x + m_crop_height, m_crop_top_left_y, m_crop_top_left_x  + m_crop_height, m_crop_top_left_y+line_width);

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