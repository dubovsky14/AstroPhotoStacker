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
                                                unsigned int box_size, unsigned int box_spacing)  {

    const unsigned short max_value = *std::max_element(image_data.brightness, image_data.brightness + image_data.width*image_data.height);
    const int step_size = box_size + box_spacing;
    m_alignment_window = alignment_window;
    for (int y = alignment_window.y_min; y < alignment_window.y_max; y += step_size) {
        for (int x = alignment_window.x_min; x < alignment_window.x_max; x += step_size) {
            if (AlignmentPointBox::is_valid_ap(image_data, x, y, box_size, box_size, max_value)) {
                m_boxes.emplace_back(image_data, x, y, box_size, box_size, max_value);
            }
        }
    }

    sort_alignment_boxes();
};

AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const AlignmentWindow &alignment_window,
                                                std::pair<int,int> box_width_range,
                                                std::pair<int,int> box_height_range,
                                                unsigned int n_boxes)   {


    const unsigned short max_value = *std::max_element(image_data.brightness, image_data.brightness + image_data.width*image_data.height);
    const unsigned int alignment_window_width = alignment_window.x_max - alignment_window.x_min;
    const unsigned int alignment_window_height = alignment_window.y_max - alignment_window.y_min;

    // not all boxes will be valid -> let's try to get n_boxes valid boxes, but also don't wanna get stuck in an infinite loop
    for (unsigned int i = 0; i < n_boxes*10; i++) {
        const int box_width  = random_uniform(box_width_range.first,  box_width_range.second);
        const int box_height = random_uniform(box_height_range.first, box_height_range.second);

        const int x = alignment_window.x_min + random_uniform(0, alignment_window_width - box_width);
        const int y = alignment_window.y_min + random_uniform(0, alignment_window_height - box_height);

        if (!fulfill_overlap_condition(m_boxes, x, y, box_width, box_height, m_maximal_overlap_between_boxes))    {
            continue;
        }

        if (AlignmentPointBox::is_valid_ap(image_data, x, y, box_width, box_height, max_value)) {
            m_boxes.emplace_back(image_data, x, y, box_width, box_height, max_value);
        }
        if (m_boxes.size() >= n_boxes) {
            break;
        }
    }

    sort_alignment_boxes();
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
    for (const AlignmentPointBox &box : m_boxes) {
        const int original_x = box.get_center_x();
        const int original_y = box.get_center_y();

        const std::tuple<int,int> prefit_shift = get_interpolated_shift(shifts, original_x, original_y);
        const int adjusted_x = original_x + get<0>(prefit_shift);
        const int adjusted_y = original_y + get<1>(prefit_shift);

        int best_x = adjusted_x;
        int best_y = adjusted_y;
        if (adjusted_x < 0 || adjusted_x >= calibrated_image.width || adjusted_y < 0 || adjusted_y >= calibrated_image.height) {
            best_x = original_x;
            best_y = original_y;
        }
        float best_chi2 = box.get_chi2(calibrated_image, best_x, best_x);

        const int max_shift_size = 20;
        for (int y_shift = -max_shift_size; y_shift <= max_shift_size; y_shift++) {
            for (int x_shift = -max_shift_size; x_shift <= max_shift_size; x_shift++) {
                if (!AlignmentPointBox::is_valid_ap(calibrated_image, adjusted_x + x_shift, adjusted_y + y_shift, box.get_box_width(), box.get_box_height(), box.get_max_value())) {
                    continue;
                }
                const float chi2 = box.get_chi2(calibrated_image, adjusted_x + x_shift, adjusted_y + y_shift);
                if (chi2 < best_chi2) {
                    best_chi2 = chi2;
                    best_x = adjusted_x + x_shift;
                    best_y = adjusted_y + y_shift;
                }
            }
        }

        const bool good_match = box.good_match(best_chi2);
        valid_boxes += good_match;

        LocalShift local_shift;
        local_shift.x = original_x;
        local_shift.y = original_y;
        local_shift.dx = good_match ?  best_x - original_x : 0;
        local_shift.dy = good_match ?  best_y - original_y : 0;
        local_shift.valid_ap = good_match;
        local_shift.score = AlignmentPointBox::get_sharpness_factor(calibrated_image, best_x, best_y, box.get_box_width(), box.get_box_height());

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