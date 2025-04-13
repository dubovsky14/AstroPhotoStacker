#pragma once

#include "../headers/AlignmentPointBox.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/LocalShift.h"
#include "../headers/AlignmentWindow.h"
#include "../headers/AlignmentSettingsSurface.h"

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
                                    const AlignmentSettingsSurface &alignment_settings);

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
             * @param maximal_allowed_overlap Maximal overlap between the boxes relative to the smaller box' area
             */
            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const AlignmentWindow &alignment_window,
                                    std::pair<int,int> box_width_range,
                                    std::pair<int,int> box_height_range,
                                    unsigned int n_boxes,
                                    float maximal_allowed_overlap);

                                    /**
             * @brief Construct a new Alignment Point Box Grid object with square boxes and uniform spacing
             *
             * @param image_data The image data
             * @param alignment_window The alignment window
             * @param alignment_points The alignment points - std::vector<std::tuple<int,int,int,int>> - x (center), y (center), box_width, box_height
             * @param sort_alignment_points Whether to sort the alignment points
             */
            AlignmentPointBoxGrid(  const MonochromeImageData &image_data,
                                    const AlignmentWindow &alignment_window,
                                    const std::vector<std::tuple<int,int,int,int>> &alignment_points,
                                    bool sort_alignment_points = true);


            /**
             * @brief Get the local shifts of the alignment point boxes.
             *
             * @param calibrated_image The calibrated image data
             * @return std::vector<std::tuple<int,int,int,int,bool>> The local shifts of the alignment point boxes: original x, original y, shift x, shift y, whether the shift is valid
             */
            std::vector<LocalShift> get_local_shifts(const MonochromeImageData &calibrated_image) const;

            const std::vector<AlignmentPointBox> &get_alignment_boxes() const {return m_boxes;};

            template<typename PixelType>
            static void draw_boxes_into_image(  const std::vector<AlignmentPointBox> &alignment_boxes,
                                                std::vector<std::vector<PixelType>> *image,
                                                int width,
                                                int height,
                                                const std::vector<int> &valid_ap_color,
                                                const std::vector<int> &invalid_ap_color)    {

                for (const AlignmentPointBox &box : alignment_boxes) {
                    const int x = box.get_center_x();
                    const int y = box.get_center_y();

                    const int x_min = std::max<int>(0, x - box.get_box_width()/2);
                    const int x_max = std::min<int>(width-1, x + box.get_box_width()/2);
                    const int y_min = std::max<int>(0, y - box.get_box_height()/2);
                    const int y_max = std::min<int>(height-1, y + box.get_box_height()/2);

                    for (int i_color = 0; i_color < 3; i_color++) {
                        for (int y = y_min; y <= y_max; y++) {
                            (*image)[i_color][y*width + x_min] = valid_ap_color[i_color]; // left
                            (*image)[i_color][y*width + x_max] = valid_ap_color[i_color]; // right
                        }
                        for (int x = x_min; x <= x_max; x++) {
                            (*image)[i_color][y_min*width + x] = valid_ap_color[i_color]; // top
                            (*image)[i_color][y_max*width + x] = valid_ap_color[i_color]; // bottom
                        }
                    }
                }
            };

            /**
             * @brief Given the provided fraction, what is the pixel brightness for which this fraction of the pixels are brighter
             */
            static unsigned short int get_brigness_for_corresponding_fraction(const MonochromeImageData &image_data, const AlignmentWindow &alignment_window, float fraction = 0.1);

            static std::vector<float> get_scaled_data_in_alignment_window(const MonochromeImageData &image_data, const AlignmentWindow &alignment_window, float scale_factor);


        private:
            std::tuple<int,int> get_interpolated_shift(const std::vector<LocalShift> &local_shifts, int x, int y)   const;

            std::vector<AlignmentPointBox> m_boxes; // x, y, box
            AlignmentWindow m_alignment_window;
            std::vector<float> m_scaled_data_reference_image_in_alignment_window;


            /**
             * @brief Sort the alignment boxes in a "snail" way around the center of the alignment window
             */
            void sort_alignment_boxes();

            static bool fulfill_overlap_condition(const std::vector<AlignmentPointBox> &boxes, int x, int y, int width, int height, float max_allowed_overlap_fraction);

    };
}