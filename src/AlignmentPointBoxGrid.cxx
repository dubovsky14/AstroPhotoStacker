#include "../headers/AlignmentPointBoxGrid.h"

#include <tuple>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

AlignmentPointBoxGrid::AlignmentPointBoxGrid(   const MonochromeImageData &image_data,
                                                const std::tuple<int,int,int,int> &alignment_window,
                                                unsigned int box_size, unsigned int box_spacing,
                                                int center_x, int center_y)  {
    const int x0 = std::get<0>(alignment_window);
    const int y0 = std::get<1>(alignment_window);
    const int x1 = std::get<2>(alignment_window);
    const int y1 = std::get<3>(alignment_window);

    const unsigned short max_value = *std::max_element(image_data.brightness, image_data.brightness + image_data.width*image_data.height);
    const int step_size = box_size + box_spacing;
    for (int y = y0; y < y1; y += step_size) {
        for (int x = x0; x < x1; x += step_size) {
            if (AlignmentPointBox::is_valid_ap(image_data, x, y, box_size, max_value)) {
                m_boxes.push_back(std::make_tuple(x, y, AlignmentPointBox(image_data, x, y, box_size, max_value)));
            }
        }
    }


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
    sort(m_boxes.begin(), m_boxes.end(), [center_x, center_y, &get_side_and_index](const std::tuple<int,int,AlignmentPointBox> &a, const std::tuple<int,int,AlignmentPointBox> &b) {
        const int x0_centered = get<0>(a) - center_x;
        const int y0_centered = get<1>(a) - center_y;
        const int x1_centered = get<0>(b) - center_x;
        const int y1_centered = get<1>(b) - center_y;

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

std::vector<std::tuple<int,int,int,int,bool>> AlignmentPointBoxGrid::get_local_shifts(const MonochromeImageData &calibrated_image) const {
    vector<tuple<int,int,int,int,bool>> shifts;
    int valid_boxes = 0;
    for (const auto &box : m_boxes) {
        const int original_x = get<0>(box);
        const int original_y = get<1>(box);
        const AlignmentPointBox &apb = get<2>(box);

        const std::tuple<int,int> prefit_shift = get_interpolated_shift(shifts, original_x, original_y);
        const int adjusted_x = original_x + get<0>(prefit_shift);
        const int adjusted_y = original_y + get<1>(prefit_shift);

        int best_x = adjusted_x;
        int best_y = adjusted_y;
        if (adjusted_x < 0 || adjusted_x >= calibrated_image.width || adjusted_y < 0 || adjusted_y >= calibrated_image.height) {
            best_x = original_x;
            best_y = original_y;
        }
        float best_chi2 = apb.get_chi2(calibrated_image, best_x, best_x);

        const int max_shift_size = 20;
        for (int y_shift = -max_shift_size; y_shift <= max_shift_size; y_shift++) {
            for (int x_shift = -max_shift_size; x_shift <= max_shift_size; x_shift++) {
                if (!AlignmentPointBox::is_valid_ap(calibrated_image, adjusted_x + x_shift, adjusted_y + y_shift, apb.get_box_size(), apb.get_max_value())) {
                    continue;
                }
                const float chi2 = apb.get_chi2(calibrated_image, adjusted_x + x_shift, adjusted_y + y_shift);
                if (chi2 < best_chi2) {
                    best_chi2 = chi2;
                    best_x = adjusted_x + x_shift;
                    best_y = adjusted_y + y_shift;
                }
            }
        }

        //cout << "Coordinates: " << original_x << ", " << original_y << " -> " << best_x << ", " << best_y << " (chi2: " << best_chi2 << ")" << endl;
        if (apb.good_match(best_chi2)) {
            shifts.push_back(std::make_tuple(original_x, original_y, best_x - original_x, best_y - original_y, true));
            valid_boxes++;
        }
        else {
            shifts.push_back(std::make_tuple(original_x, original_y, 0, 0, false));
        }
    }

    cout << "Number of found alignment points: " << valid_boxes << " / " << m_boxes.size() << endl;

    return shifts;
};

std::tuple<int,int> AlignmentPointBoxGrid::get_interpolated_shift(const std::vector<std::tuple<int,int,int,int,bool>> &local_shifts, int x, int y)   {
    vector<std::tuple<int,int,float>> shifts_distances;
    if (local_shifts.size() == 0) {
        return std::make_tuple(0, 0);
    }
    const int n_points = 4;
    for (const auto &shift : local_shifts) {
        if (get<4>(shift) == false) {
            continue;
        }
        const int x0 = get<0>(shift);
        const int y0 = get<1>(shift);
        const int x1 = get<2>(shift);
        const int y1 = get<3>(shift);

        const float distance = sqrt((x - x0)*(x - x0) + (y - y0)*(y - y0));
        if (shifts_distances.size() < n_points) {
            shifts_distances.push_back(std::make_tuple(x1, y1, distance));
            sort(shifts_distances.begin(), shifts_distances.end(), [](const auto &a, const auto &b) {
                return get<2>(a) < get<2>(b);
            });
        }
        else {
            if (distance < get<2>(shifts_distances.back())) {
                shifts_distances.back() = std::make_tuple(x1, y1, distance);
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