#include "../headers/ImagePreviewCometSelectionTool.h"

#include "../../headers/StarFinder.h"

#include<algorithm>

using namespace std;


ImagePreviewCometSelectionTool::ImagePreviewCometSelectionTool(wxFrame *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {
};

void ImagePreviewCometSelectionTool::update_preview_bitmap()   {
    wxImage image_wx = get_updated_wximage();

    plot_stars(&image_wx);

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};



std::vector<std::pair<float,float>> ImagePreviewCometSelectionTool::calculate_star_positions() {
    if (m_original_image.size() == 0) {
        return {};
    }

    vector<float> monochrome_image(m_original_image[0].size(), 0.0f);

    for (size_t i_color = 0; i_color < m_original_image.size(); i_color++) {
        for (size_t i_pixel = 0; i_pixel < m_original_image[i_color].size(); i_pixel++) {
            monochrome_image[i_pixel] += static_cast<float>(m_original_image[i_color][i_pixel]);
        }
    }

    std::vector<std::tuple<float,float,int>> stars = AstroPhotoStacker::get_stars(  monochrome_image.data(),
                                                                m_image_resize_tool.get_width_original(),
                                                                m_image_resize_tool.get_height_original(),
                                                                0.001f);

    std::sort(stars.begin(), stars.end(), [](const auto &a, const auto &b) {
        return std::get<2>(a) > std::get<2>(b); // Sort by size in descending order
    });

    std::vector<std::pair<float,float>> star_positions;
    for (const auto& star : stars) {
        const float x = std::get<0>(star);
        const float y = std::get<1>(star);
        const int size = std::get<2>(star);
        if (size < 9) {
            continue; // Skip small stars
        }
        star_positions.push_back(std::make_pair(x, y));
    }
    return star_positions;
}

void ImagePreviewCometSelectionTool::on_update_original_image() {
    m_star_positions = calculate_star_positions();
};

void ImagePreviewCometSelectionTool::plot_stars(wxImage *wx_image) {
    if (m_comet_position.first < 0 || m_comet_position.second < 0) {
        return;
    }
    m_additional_layers_data = std::vector<std::vector<PixelType>>(3, std::vector<PixelType>(m_image_resize_tool.get_width_original()*m_image_resize_tool.get_height_original(), -1));

    // Plot comet position
    add_circle_to_additional_layers(m_comet_position.first, m_comet_position.second, 15.0f, 10.0f, {255, 0, 0});
};

void ImagePreviewCometSelectionTool::add_circle_to_additional_layers(float center_x, float center_y, float radius_outer, float radius_inner, std::array<int,3> color) {
    const int width = m_image_resize_tool.get_width_original();
    const int height = m_image_resize_tool.get_height_original();

    const int radius_outer_squared = static_cast<int>(radius_outer * radius_outer);
    const int radius_inner_squared = static_cast<int>(radius_inner * radius_inner);

    const int x_start = static_cast<int>(max(0.0f, center_x - radius_outer));
    const int x_end = static_cast<int>(min(static_cast<float>(width - 1), center_x + radius_outer));
    const int y_start = static_cast<int>(max(0.0f, center_y - radius_outer));
    const int y_end = static_cast<int>(min(static_cast<float>(height - 1), center_y + radius_outer));

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            const int dx = x - static_cast<int>(center_x);
            const int dy = y - static_cast<int>(center_y);
            if ((dx * dx + dy * dy <= radius_outer_squared) && (dx * dx + dy * dy >= radius_inner_squared)) {
                const int index = y * width + x;
                m_additional_layers_data[0][index] = color[0];
                m_additional_layers_data[1][index] = color[1];
                m_additional_layers_data[2][index] = color[2];
            }
        }
    }
};

void ImagePreviewCometSelectionTool::bind_comet_selection_events() {
    m_preview_bitmap->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent &event) {
        if (!image_loaded()) {
            return;
        }

        // Get the mouse position in screen coordinates
        wxPoint screen_pos = event.GetPosition();

        // Check if the mouse is over the wxGenericStaticBitmap
        if (wxRect(m_preview_bitmap->GetSize()).Contains(screen_pos)) {
            // Calculate the relative position of the mouse within the wxGenericStaticBitmap
            wxSize bitmapSize = m_preview_bitmap->GetSize();
            float relative_x = static_cast<float>(screen_pos.x) / bitmapSize.GetWidth();
            float relative_y = static_cast<float>(screen_pos.y) / bitmapSize.GetHeight();

            const float x_original = m_image_resize_tool.get_original_coordinate_x(relative_x);
            const float y_original = m_image_resize_tool.get_original_coordinate_y(relative_y);

            int closest_star_index = get_closest_star_index(x_original, y_original);
            if (closest_star_index >= 0) {
                m_comet_position = m_star_positions[closest_star_index];
            } else {
                m_comet_position = std::make_pair(x_original, y_original);
            }

            this->update_preview_bitmap();
        }
    });
};