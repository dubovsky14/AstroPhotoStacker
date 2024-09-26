#pragma once

#include "../headers/AlignmentPointBox.h"
#include "../headers/MonochromeImageData.h"

#include <tuple>
#include <vector>

namespace AstroPhotoStacker {
    class AlignmentPointBoxGrid {
        public:
            AlignmentPointBoxGrid() = delete;

            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const std::tuple<int,int,int,int> &alignment_window,
                                    unsigned int box_size, unsigned int box_spacing, int center_x, int center_y);

            /**
             * @brief Get the local shifts of the alignment point boxes.
             *
             * @param calibrated_image The calibrated image data
             * @return std::vector<std::tuple<int,int,int,int,bool>> The local shifts of the alignment point boxes: original x, original y, shift x, shift y, whether the shift is valid
             */
            std::vector<std::tuple<int,int,int,int,bool>> get_local_shifts(const MonochromeImageData &calibrated_image) const;

            const std::vector<std::tuple<int,int,AlignmentPointBox>> &get_alignment_boxes() const {return m_boxes;};


        private:
            static std::tuple<int,int> get_interpolated_shift(const std::vector<std::tuple<int,int,int,int,bool>> &local_shifts, int x, int y);

            std::vector<std::tuple<int,int,AlignmentPointBox>> m_boxes; // x, y, box

    };
}