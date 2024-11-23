#pragma once

#include "../headers/AlignmentPointBox.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/LocalShift.h"
#include "../headers/AlignmentWindow.h"

#include <tuple>
#include <vector>

namespace AstroPhotoStacker {
    class AlignmentPointBoxGrid {
        public:
            AlignmentPointBoxGrid() = delete;

            /**
             * @brief Construct a new Alignment Point Box Grid object with square boxes and uniform spacing
             */
            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const AlignmentWindow &alignment_window,
                                    unsigned int box_size, unsigned int box_spacing);


            /**
             * @brief Construct a new Alignment Point Box Grid object with square boxes and uniform spacing
             *
             * @param image_data The image data
             * @param alignment_window The alignment window
             * @param box_width_range The range of the box width in pixels - std::pair<float,float> - min, max
             * @param box_height_range The range of the box height in pixels - std::pair<float,float> - min, max
             * @param n_boxes The number of boxes
             */
            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const AlignmentWindow &alignment_window,
                                    std::pair<int,int> box_width_range,
                                    std::pair<int,int> box_height_range,
                                    unsigned int n_boxes);

            /**
             * @brief Get the local shifts of the alignment point boxes.
             *
             * @param calibrated_image The calibrated image data
             * @return std::vector<std::tuple<int,int,int,int,bool>> The local shifts of the alignment point boxes: original x, original y, shift x, shift y, whether the shift is valid
             */
            std::vector<LocalShift> get_local_shifts(const MonochromeImageData &calibrated_image) const;

            const std::vector<std::tuple<int,int,AlignmentPointBox>> &get_alignment_boxes() const {return m_boxes;};


        private:
            std::tuple<int,int> get_interpolated_shift(const std::vector<LocalShift> &local_shifts, int x, int y)   const;

            std::vector<std::tuple<int,int,AlignmentPointBox>> m_boxes; // x, y, box
            AlignmentWindow m_alignment_window;

            /**
             * @brief Sort the alignment boxes in a "snail" way around the center of the alignment window
             */
            void sort_alignment_boxes();

    };
}