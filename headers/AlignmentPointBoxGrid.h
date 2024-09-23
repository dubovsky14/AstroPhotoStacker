#pragma once

#include "../headers/AlignmentPointBox.h"
#include "../headers/MonochromeImageData.h"

#include <tuple>
#include <vector>

namespace AstroPhotoStacker {
    class AlignmentPointBoxGrid {
        public:
            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const std::tuple<int,int,int,int> &alignment_window,
                                    unsigned int box_size, unsigned int box_spacing, int center_x, int center_y);


            std::vector<std::tuple<int,int,int,int>> get_local_shifts(const MonochromeImageData &calibrated_image) const;


        private:
            std::vector<std::tuple<int,int,AlignmentPointBox>> m_boxes; // x, y, box

    };
}