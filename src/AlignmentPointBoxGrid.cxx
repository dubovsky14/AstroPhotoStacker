#include "../headers/AlignmentPointBoxGrid.h"
#include "../headers/Common.h"

#include <tuple>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const AlignmentWindow &alignment_window,
                                                const AlignmentSettingsSurface &alignment_settings) {

    const bool regular_grid = alignment_settings.get_regular_grid();
    const unsigned int n_boxes = alignment_settings.get_number_of_boxes();
    const float maximal_allowed_overlap = alignment_settings.get_max_overlap_between_boxes();

    // #TODO: clean up this mess
    const int alignment_window_width = alignment_window.x_max - alignment_window.x_min;
    const int alignment_window_height = alignment_window.y_max - alignment_window.y_min;

    const int min_box_width = max(20, alignment_window_width/60);
    const int max_box_width = max(400, alignment_window_width/5);
    const int min_box_height = max(20, alignment_window_height/60);
    const int max_box_height = max(400, alignment_window_height/5);
    const std::pair<int,int> box_width_range = {min_box_width, max_box_width};
    const std::pair<int,int> box_height_range = {min_box_height, max_box_height};

    const float scale_factor = 1./get_brightness_for_corresponding_fraction(image_data, alignment_window, 0.1);
    m_scaled_data_reference_image_in_alignment_window = get_scaled_data_in_alignment_window(image_data, alignment_window, scale_factor);
    const float max_value = *std::max_element(m_scaled_data_reference_image_in_alignment_window.begin(), m_scaled_data_reference_image_in_alignment_window.end());

    if (regular_grid)   {
        const int alignment_window_area = (alignment_window.x_max - alignment_window.x_min)*(alignment_window.y_max - alignment_window.y_min);

        const int box_area = alignment_window_area/n_boxes;
        const int box_size = sqrt(box_area)*0.8;
        const int box_spacing = box_size/4;

        const int step_size = box_size + box_spacing;
        m_alignment_window = alignment_window;
        const float scale_factor = 1./get_brightness_for_corresponding_fraction(image_data, alignment_window, 0.1);
        m_scaled_data_reference_image_in_alignment_window = get_scaled_data_in_alignment_window(image_data, alignment_window, scale_factor);
        const float max_value = *std::max_element(m_scaled_data_reference_image_in_alignment_window.begin(), m_scaled_data_reference_image_in_alignment_window.end());
        for (int y = alignment_window.y_min; y < alignment_window.y_max; y += step_size) {
            for (int x = alignment_window.x_min; x < alignment_window.x_max; x += step_size) {
                int xa = x - alignment_window.x_min;
                int ya = y - alignment_window.y_min;
                if (AlignmentPointBox::is_valid_ap(&m_scaled_data_reference_image_in_alignment_window, alignment_window, xa, ya, box_size, box_size, max_value)) {
                    m_boxes.emplace_back(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_size, box_size, max_value);
                }
            }
        }
    }
    else {
        const unsigned int alignment_window_width = alignment_window.x_max - alignment_window.x_min;
        const unsigned int alignment_window_height = alignment_window.y_max - alignment_window.y_min;
        m_alignment_window = alignment_window;

        // not all boxes will be valid -> let's try to get n_boxes valid boxes, but also don't wanna get stuck in an infinite loop
        for (unsigned int i = 0; i < n_boxes*10; i++) {
            const int box_width  = random_uniform(box_width_range.first,  box_width_range.second);
            const int box_height = random_uniform(box_height_range.first, box_height_range.second);

            const int x = alignment_window.x_min + random_uniform(0, alignment_window_width - box_width);
            const int y = alignment_window.y_min + random_uniform(0, alignment_window_height - box_height);

            if (!fulfill_overlap_condition(m_boxes, x, y, box_width, box_height, maximal_allowed_overlap))    {
                continue;
            }

            int xa = x - alignment_window.x_min;
            int ya = y - alignment_window.y_min;
            if (AlignmentPointBox::is_valid_ap(&m_scaled_data_reference_image_in_alignment_window, alignment_window, xa, ya, box_width, box_height, max_value)) {
                m_boxes.emplace_back(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_width, box_height, max_value);
            }
            if (m_boxes.size() >= n_boxes) {
                break;
            }
        }
    }
    sort_alignment_boxes();
};

AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const AlignmentWindow &alignment_window,
                                                unsigned int box_size, unsigned int box_spacing)  {
    const int step_size = box_size + box_spacing;
    m_alignment_window = alignment_window;
    const float scale_factor = 1./get_brightness_for_corresponding_fraction(image_data, alignment_window, 0.1);
    m_scaled_data_reference_image_in_alignment_window = get_scaled_data_in_alignment_window(image_data, alignment_window, scale_factor);
    const float max_value = *std::max_element(m_scaled_data_reference_image_in_alignment_window.begin(), m_scaled_data_reference_image_in_alignment_window.end());
    for (int y = alignment_window.y_min; y < alignment_window.y_max; y += step_size) {
        for (int x = alignment_window.x_min; x < alignment_window.x_max; x += step_size) {
            int xa = x - alignment_window.x_min;
            int ya = y - alignment_window.y_min;
            if (AlignmentPointBox::is_valid_ap(&m_scaled_data_reference_image_in_alignment_window, alignment_window, xa, ya, box_size, box_size, max_value)) {
                m_boxes.emplace_back(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_size, box_size, max_value);
            }
        }
    }

    sort_alignment_boxes();
};

AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const AlignmentWindow &alignment_window,
                                                std::pair<int,int> box_width_range,
                                                std::pair<int,int> box_height_range,
                                                unsigned int n_boxes,
                                                float maximal_allowed_overlap)   {


    const int alignment_window_area = (alignment_window.x_max - alignment_window.x_min)*(alignment_window.y_max - alignment_window.y_min);

    const int box_area = alignment_window_area/n_boxes;
    const int box_size = sqrt(box_area)*0.8;
    const int box_spacing = box_size/4;

    const int step_size = box_size + box_spacing;
    m_alignment_window = alignment_window;
    const float scale_factor = 1./get_brightness_for_corresponding_fraction(image_data, alignment_window, 0.1);
    m_scaled_data_reference_image_in_alignment_window = get_scaled_data_in_alignment_window(image_data, alignment_window, scale_factor);
    const float max_value = *std::max_element(m_scaled_data_reference_image_in_alignment_window.begin(), m_scaled_data_reference_image_in_alignment_window.end());
    for (int y = alignment_window.y_min; y < alignment_window.y_max; y += step_size) {
        for (int x = alignment_window.x_min; x < alignment_window.x_max; x += step_size) {
            int xa = x - alignment_window.x_min;
            int ya = y - alignment_window.y_min;
            if (AlignmentPointBox::is_valid_ap(&m_scaled_data_reference_image_in_alignment_window, alignment_window, xa, ya, box_size, box_size, max_value)) {
                m_boxes.emplace_back(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_size, box_size, max_value);
            }
        }
    }

    sort_alignment_boxes();
};

AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const AlignmentWindow &alignment_window,
                                                const std::vector<std::tuple<int,int,int,int>> &alignment_points,
                                                bool sort_alignment_points)   {

    m_alignment_window = alignment_window;
    const float scale_factor = 1./get_brightness_for_corresponding_fraction(image_data, alignment_window, 0.1);
    m_scaled_data_reference_image_in_alignment_window = get_scaled_data_in_alignment_window(image_data, alignment_window, scale_factor);
    const float max_value = *std::max_element(m_scaled_data_reference_image_in_alignment_window.begin(), m_scaled_data_reference_image_in_alignment_window.end());

    for (const auto &alignment_point : alignment_points) {
        const int x = std::get<0>(alignment_point);
        const int y = std::get<1>(alignment_point);
        const int box_width = std::get<2>(alignment_point);
        const int box_height = std::get<3>(alignment_point);

        if (AlignmentPointBox::is_valid_ap(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_width, box_height, max_value)) {
            m_boxes.emplace_back(&m_scaled_data_reference_image_in_alignment_window, alignment_window, x, y, box_width, box_height, max_value);
        }
    }

    if (sort_alignment_points) {
        sort_alignment_boxes();
    }
};


void AlignmentPointBoxGrid::sort_alignment_boxes()    {
    auto get_side_and_index = [](int x_centered, int y_centered) {
        const int max_coordinate = max(abs(x_centered), abs(y_centered));

        if (y_centered == max_coordinate) {
            return std::make_tuple(0, x_centered);
        }
        if (x_centered == max_coordinate) {
            return std::make_tuple(1, -y_centered);
        }
        if (y_centered == -max_coordinate) {
            return std::make_tuple(2, -x_centered);
        }
        if (x_centered == -max_coordinate) {
            return std::make_tuple(3, y_centered);
        }
        return std::make_tuple(-1, -1);
    };

    // sort them in a "snail" way
    const int center_x = (m_alignment_window.x_min + m_alignment_window.x_max)/2;
    const int center_y = (m_alignment_window.y_min + m_alignment_window.y_max)/2;
    sort(m_boxes.begin(), m_boxes.end(), [center_x, center_y, &get_side_and_index](const AlignmentPointBox &a, const AlignmentPointBox &b) {
        const int x0_centered = a.get_center_x() - center_x;
        const int y0_centered = a.get_center_y() - center_y;
        const int x1_centered = b.get_center_x() - center_x;
        const int y1_centered = b.get_center_y() - center_y;

        int max_0 = max(abs(x0_centered), abs(y0_centered));
        int max_1 = max(abs(x1_centered), abs(y1_centered));
        return max_0 < max_1;
        if (max_0 != max_1) {
            return max_0 < max_1;
        }

        const auto [side_0, index_0] = get_side_and_index(x0_centered, y0_centered);
        const auto [side_1, index_1] = get_side_and_index(x1_centered, y1_centered);

        if (side_0 != side_1) {
            return side_0 < side_1;
        }
        return index_0 < index_1;
    });
};


std::vector<LocalShift> AlignmentPointBoxGrid::get_local_shifts(const MonochromeImageData &calibrated_image) const {
    vector<LocalShift> shifts;
    int valid_boxes = 0;
    const int alignment_window_width = m_alignment_window.x_max - m_alignment_window.x_min;
    const int alignment_window_height = m_alignment_window.y_max - m_alignment_window.y_min;

    const float scale_factor = 1./get_brightness_for_corresponding_fraction(calibrated_image, m_alignment_window, 0.1);
    const vector<float> scaled_values_in_alignment_window = get_scaled_data_in_alignment_window(calibrated_image, m_alignment_window, scale_factor);
    for (const AlignmentPointBox &box : m_boxes) {
        const int original_x = box.get_center_x();
        const int original_y = box.get_center_y();

        const std::tuple<int,int> prefit_shift = get_interpolated_shift(shifts, original_x, original_y);
        const int adjusted_x = original_x - m_alignment_window.x_min + get<0>(prefit_shift);
        const int adjusted_y = original_y - m_alignment_window.y_min + get<1>(prefit_shift);

        int best_x = adjusted_x;
        int best_y = adjusted_y;
        if (adjusted_x < 0 || adjusted_x >= alignment_window_width || adjusted_y < 0 || adjusted_y >= alignment_window_height) {
            best_x = original_x - m_alignment_window.x_min;
            best_y = original_y - m_alignment_window.y_min;
        }
        float best_chi2 = std::numeric_limits<float>::max();

        const int max_shift_size = 20;
        for (int y_shift = -max_shift_size; y_shift <= max_shift_size; y_shift++) {
            for (int x_shift = -max_shift_size; x_shift <= max_shift_size; x_shift++) {
                if (!AlignmentPointBox::is_valid_ap(&scaled_values_in_alignment_window, m_alignment_window, adjusted_x + x_shift, adjusted_y + y_shift, box.get_box_width(), box.get_box_height(), box.get_max_value())) {
                    continue;
                }
                const float chi2 = box.get_chi2(&scaled_values_in_alignment_window, adjusted_x + x_shift, adjusted_y + y_shift);
                if (chi2 < best_chi2) {
                    best_chi2 = chi2;
                    best_x = adjusted_x + x_shift;
                    best_y = adjusted_y + y_shift;
                }
            }
        }

        const bool good_match = box.good_match(best_chi2);
        valid_boxes += good_match;

        // shift best_x and best_y back to the original image coordinates
        best_x += m_alignment_window.x_min;
        best_y += m_alignment_window.y_min;

        LocalShift local_shift;
        local_shift.x = original_x;
        local_shift.y = original_y;
        local_shift.dx = good_match ?  best_x - original_x : 0;
        local_shift.dy = good_match ?  best_y - original_y : 0;
        local_shift.valid_ap = good_match;
        local_shift.score = AlignmentPointBox::get_sharpness_factor(&scaled_values_in_alignment_window, m_alignment_window, best_x, best_y, box.get_box_width(), box.get_box_height());

        shifts.push_back(local_shift);
    }
    cout << "Number of found alignment points: " << valid_boxes << " / " << m_boxes.size() << endl;

    return shifts;
};

std::tuple<int,int> AlignmentPointBoxGrid::get_interpolated_shift(const vector<LocalShift> &local_shifts, int x, int y) const  {
    vector<std::tuple<int,int,float>> shifts_distances;
    if (local_shifts.size() == 0) {
        return std::make_tuple(0, 0);
    }

    if (x < m_alignment_window.x_min || x >= m_alignment_window.x_max || y < m_alignment_window.y_min || y >= m_alignment_window.y_max) {
        return std::make_tuple(0, 0);
    }

    const int n_points = 4;
    for (const auto &shift : local_shifts) {
        if (shift.valid_ap == false) {
            continue;
        }
        const int x0 = shift.x;
        const int y0 = shift.y;
        const int dx = shift.dx;
        const int dy = shift.dy;

        const float distance = sqrt((x - x0)*(x - x0) + (y - y0)*(y - y0));
        if (shifts_distances.size() < n_points) {
            shifts_distances.push_back(std::make_tuple(dx, dy, distance));
            sort(shifts_distances.begin(), shifts_distances.end(), [](const auto &a, const auto &b) {
                return get<2>(a) < get<2>(b);
            });
        }
        else {
            if (distance < get<2>(shifts_distances.back())) {
                shifts_distances.back() = std::make_tuple(dx, dy, distance);
                sort(shifts_distances.begin(), shifts_distances.end(), [](const auto &a, const auto &b) {
                    return get<2>(a) < get<2>(b);
                });
            }
        }
    }

    float sum_x = 0;
    float sum_y = 0;
    float sum_weights = 0;
    for (const auto &shift : shifts_distances) {
        const int x1 = get<0>(shift);
        const int y1 = get<1>(shift);

        if (get<2>(shift) == 0) {
            return std::make_tuple(x1, y1);
        }

        const float weight = 1.0f / get<2>(shift);
        sum_x += x1 * weight;
        sum_y += y1 * weight;
        sum_weights += weight;
    }

    if (sum_weights == 0) {
        return std::make_tuple(0, 0);
    }

    return std::make_tuple(sum_x / sum_weights, sum_y / sum_weights);
};

bool AlignmentPointBoxGrid::fulfill_overlap_condition(const std::vector<AlignmentPointBox> &boxes, int x, int y, int width, int height, float max_allowed_overlap_fraction)     {
    for (const AlignmentPointBox &box : boxes) {
        const int old_box_x_min = box.get_center_x() - box.get_box_width()/2;
        const int old_box_x_max = box.get_center_x() + box.get_box_width()/2;
        const int old_box_y_min = box.get_center_y() - box.get_box_height()/2;
        const int old_box_y_max = box.get_center_y() + box.get_box_height()/2;
        const unsigned int old_box_area = box.get_box_width() * box.get_box_height();

        const int new_box_x_min = x;
        const int new_box_x_max = x + width;
        const int new_box_y_min = y;
        const int new_box_y_max = y + height;
        const unsigned int new_box_area = width * height;

        auto get_one_dimension_overlap = [](int a_min, int a_max, int b_min, int b_max) -> std::pair<int,int> {
            if (a_min > b_min) {
                swap(a_min, b_min);
                swap(a_max, b_max);
            }

            if (a_max < b_min) {
                return {-1, -1};
            }

            return {b_min, min(a_max, b_max)};
        };

        const auto [x_overlap_start, x_overlap_end] = get_one_dimension_overlap(old_box_x_min, old_box_x_max, new_box_x_min, new_box_x_max);
        const auto [y_overlap_start, y_overlap_end] = get_one_dimension_overlap(old_box_y_min, old_box_y_max, new_box_y_min, new_box_y_max);

        if (x_overlap_start == -1 || y_overlap_start == -1) {
            continue;
        }

        const unsigned int overlap_area = (x_overlap_end - x_overlap_start) * (y_overlap_end - y_overlap_start);

        if (overlap_area > max_allowed_overlap_fraction * min(old_box_area, new_box_area)) {
            return false;
        }
    }
    return true;
};

PixelType AlignmentPointBoxGrid::get_brightness_for_corresponding_fraction(  const MonochromeImageData &image_data,
                                                                                    const AlignmentWindow &alignment_window,
                                                                                    float fraction) {

    vector<unsigned int> histogram(65536, 0);
    for (int y = alignment_window.y_min; y < alignment_window.y_max; y++) {
        for (int x = alignment_window.x_min; x < alignment_window.x_max; x++) {
            const PixelType brightness = image_data.brightness[y*image_data.width + x];
            histogram[brightness]++;
        }
    }

    const unsigned int total_pixels = (alignment_window.y_max - alignment_window.y_min) * (alignment_window.x_max - alignment_window.x_min);
    const unsigned int targeted_n_pixels_above_threshold = fraction * total_pixels;
    unsigned int sum = 0;
    for (int i = histogram.size() - 1; i >= 0; i--) {
        sum += histogram[i];
        if (sum >= targeted_n_pixels_above_threshold) {
            return i;
        }
    }
    return 0;
};

std::vector<float> AlignmentPointBoxGrid::get_scaled_data_in_alignment_window(  const MonochromeImageData &image_data,
                                                                                const AlignmentWindow &alignment_window,
                                                                                float scale_factor) {
    vector<float> scaled_data;
    const int alignment_window_width  = alignment_window.x_max - alignment_window.x_min;
    const int alignment_window_height = alignment_window.y_max - alignment_window.y_min;
    scaled_data.resize(alignment_window_width*alignment_window_height);
    for (int i_y = alignment_window.y_min; i_y < alignment_window.y_max; i_y++) {
        for (int i_x = alignment_window.x_min; i_x < alignment_window.x_max; i_x++) {
            const int index_original = i_y*image_data.width + i_x;
            const int index_scaled = (i_y - alignment_window.y_min)*alignment_window_width + (i_x - alignment_window.x_min);
            scaled_data[index_scaled] = scale_factor*image_data.brightness[index_original];
        }
    }
    return scaled_data;
};