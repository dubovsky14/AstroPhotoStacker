#pragma once

#include <vector>
#include <tuple>
#include <memory>

#include "../headers/KDTree.h"


namespace AstroPhotoStacker {
    class LocalShiftsHandler {
        public:
            LocalShiftsHandler() = delete;

            LocalShiftsHandler(const std::vector<std::tuple<int, int, int, int, bool>> &shifts);

            const std::vector<std::tuple<int, int, int, int, bool>> &get_shifts() const {
                return m_shifts;
            };

            bool calculate_shifted_coordinates(int x, int y, int *shifted_x, int *shifted_y) const;

        private:
            std::vector<std::tuple<int, int, int, int, bool>> m_shifts;

            std::unique_ptr<KDTree<int,2,std::tuple<int,int,bool>>> m_kd_tree_shifts = nullptr;

    };
}