#include "../headers/ImagePreviewCometSelectionTool.h"

#include "../../headers/StarFinder.h"

#include<algorithm>

using namespace std;


ImagePreviewCometSelectionTool::ImagePreviewCometSelectionTool(wxWindow *parent, int width, int height, int max_value, bool use_color_interpolation) :
    ImagePreview(parent, width, height, max_value, use_color_interpolation) {
    bind_comet_selection_events();
};

void ImagePreviewCometSelectionTool::update_preview_bitmap()   {
    wxImage image_wx = get_updated_wximage();

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    m_preview_bitmap->SetBitmap(bitmap);
};


void ImagePreviewCometSelectionTool::set_comet_position(float x, float y) {
    m_comet_position = std::make_pair(x, y);
    plot_stars();
};



std::vector<std::pair<float,float>> ImagePreviewCometSelectionTool::calculate_star_positions() {
    if (m_original_image.size() == 0) {
        return {};
    }

    vector<float> monochrome_image(m_original_image[0].size(), 0.0f);
    for (size_t i_color = 0; i_color < m_original_image.size(); i_color++) {
        for (size_t i_pixel = 0; i_pixel < m_original_image[i_color].size(); i_pixel++) {
            monochrome_image.at(i_pixel) += static_cast<float>(m_original_image[i_color][i_pixel])/3.0f;
        }
    }
    const float threshold = AstroPhotoStacker::get_threshold_value(monochrome_image.data(), monochrome_image.size(), 0.0005);
    std::vector<std::tuple<float,float,int>> stars = AstroPhotoStacker::get_stars(  monochrome_image.data(),
                                                                m_image_resize_tool.get_width_original(),
                                                                m_image_resize_tool.get_height_original(),
                                                                threshold);

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
    remove_layer(c_comet_layer_name);
    m_star_positions = calculate_star_positions();
    m_comet_position = std::make_pair(-1.0f, -1.0f);
    update_preview_bitmap();
};

void ImagePreviewCometSelectionTool::plot_stars() {
    if (m_comet_position.first < 0 || m_comet_position.second < 0) {
        return;
    }

    // Plot comet position
    add_circle_to_additional_layers(m_comet_position.first, m_comet_position.second, 15.0f, 10.0f, {255, 0, 0});
};

void ImagePreviewCometSelectionTool::add_circle_to_additional_layers(float center_x, float center_y, float radius_outer, float radius_inner, std::array<int,3> color) {
    const int radius_outer_squared = static_cast<int>(radius_outer * radius_outer);
    const int radius_inner_squared = static_cast<int>(radius_inner * radius_inner);

    add_layer(c_comet_layer_name, [=](std::vector<std::vector<PixelType>> *layer_data, int width, int height) {
        const int x_start = static_cast<int>(max(0.0f, center_x - radius_outer));
        const int x_end = static_cast<int>(min(static_cast<float>(width - 1), center_x + radius_outer));
        const int y_start = static_cast<int>(max(0.0f, center_y - radius_outer));
        const int y_end = static_cast<int>(min(static_cast<float>(height - 1), center_y + radius_outer));

        for (int channel = 0; channel < 3; ++channel) {
            for (int y = y_start; y <= y_end; ++y) {
                for (int x = x_start; x <= x_end; ++x) {
                    const int dx = x - static_cast<int>(center_x);
                    const int dy = y - static_cast<int>(center_y);
                    const int distance_squared = dx * dx + dy * dy;

                    if (distance_squared <= radius_outer_squared && distance_squared >= radius_inner_squared) {
                        const int index = y * width + x;
                        (*layer_data)[channel][index] = color[channel];
                    }
                }
            }
        }
    });
};

void ImagePreviewCometSelectionTool::bind_comet_selection_events() {
    m_preview_bitmap->Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent &event) {
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

            plot_stars();
        }
    });
};

int ImagePreviewCometSelectionTool::get_closest_star_index(float x, float y) const {
    int closest_index = -1;
    float min_distance_squared = std::numeric_limits<float>::max();
    for (size_t i = 0; i < m_star_positions.size(); ++i) {
        const float dx = m_star_positions[i].first - x;
        const float dy = m_star_positions[i].second - y;
        const float distance_squared = dx * dx + dy * dy;
        if (distance_squared < min_distance_squared) {
            min_distance_squared = distance_squared;
            closest_index = static_cast<int>(i);
        }
    }
    return closest_index;
};