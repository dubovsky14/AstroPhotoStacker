#include "../headers/AlignmentPointBoxGrid.h"

#include <tuple>
#include <vector>
#include <algorithm>

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
};

