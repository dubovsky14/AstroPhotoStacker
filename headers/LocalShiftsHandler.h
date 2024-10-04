#pragma once

#include <vector>
#include <tuple>

#include "../headers/KDTree.h"
#include "../headers/LocalShift.h"


namespace AstroPhotoStacker {
    class LocalShiftsHandler {
        public:
            LocalShiftsHandler() = default;

            LocalShiftsHandler(const LocalShiftsHandler &local_shifts_handler) = default;

            LocalShiftsHandler(const std::vector<LocalShift> &shifts);

            const std::vector<LocalShift> &get_shifts() const {
                return m_shifts;
            };

            bool calculate_shifted_coordinates(int x, int y, int *shifted_x, int *shifted_y) const;

            inline bool empty() const { return m_empty; };

            // for debugging
            void draw_ap_boxes_into_image(std::vector<std::vector<unsigned short>> *image, int width, int height, int boxsize, const std::vector<int> &valid_ap_color, const std::vector<int> &invalid_ap_color, int global_shift_x = 0, int global_shift_y = 0) const;

        private:
            std::vector<LocalShift> m_shifts;

            KDTree<int,2,std::tuple<int,int,bool>> m_kd_tree_shifts;

            bool m_empty = true;

    };
}