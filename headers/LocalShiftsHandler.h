#pragma once

#include <vector>
#include <tuple>

#include "../headers/KDTree.h"


namespace AstroPhotoStacker {
    class LocalShiftsHandler {
        public:
            LocalShiftsHandler() = default;

            LocalShiftsHandler(const LocalShiftsHandler &local_shifts_handler) = default;

            LocalShiftsHandler(const std::vector<std::tuple<int, int, int, int, bool>> &shifts);

            const std::vector<std::tuple<int, int, int, int, bool>> &get_shifts() const {
                return m_shifts;
            };

            bool calculate_shifted_coordinates(int x, int y, int *shifted_x, int *shifted_y) const;

            inline bool empty() const { return m_empty; };

        private:
            std::vector<std::tuple<int, int, int, int, bool>> m_shifts;

            KDTree<int,2,std::tuple<int,int,bool>> m_kd_tree_shifts;

            bool m_empty = true;

    };
}