#pragma once

#include <vector>
#include <tuple>
#include <string>

#include "../headers/KDTreeWithBuffer.h"
#include "../headers/LocalShift.h"
#include "../headers/PixelType.h"


namespace AstroPhotoStacker {
    class LocalShiftsHandler {
        public:
            LocalShiftsHandler() = default;

            LocalShiftsHandler(const LocalShiftsHandler &local_shifts_handler) = default;

            LocalShiftsHandler(const std::vector<LocalShift> &shifts);

            LocalShiftsHandler(const std::string &string_from_alignment_file);

            const std::vector<LocalShift> &get_shifts() const {
                return m_shifts;
            };

            bool calculate_shifted_coordinates(float *x, float *y, float *score = nullptr);

            inline bool empty() const { return m_empty; };

            // for debugging
            void draw_ap_boxes_into_image(std::vector<std::vector<PixelType>> *image, int width, int height, int boxsize, const std::vector<int> &valid_ap_color, const std::vector<int> &invalid_ap_color, int global_shift_x = 0, int global_shift_y = 0) const;

            // for storing alignment boxes in alignment text file
            std::string to_string() const;

        private:
            std::vector<LocalShift> m_shifts;

            void initialize(const std::vector<LocalShift> &shifts);

            KDTreeWithBuffer<int,2,std::tuple<int,int,bool,float>> m_kd_tree_shifts;  // dx, dy, valid_ap, score

            bool m_empty = true;
    };
}