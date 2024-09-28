#include "../headers/LocalShiftsHandler.h"

#include <vector>
#include <tuple>

using namespace AstroPhotoStacker;
using namespace std;

LocalShiftsHandler::LocalShiftsHandler(const std::vector<std::tuple<int, int, int, int, bool>> &shifts) : m_shifts(shifts) {
    m_kd_tree_shifts = std::make_unique<KDTree<int,2,std::tuple<int,int,bool>>>();
    for (const auto &shift : shifts) {
        const std::vector<int> coordinate = {std::get<0>(shift), std::get<1>(shift)};
        const std::tuple<int,int,bool> value = std::tuple<int,int,bool>(std::get<2>(shift), std::get<3>(shift), std::get<4>(shift));

        m_kd_tree_shifts->add_point(coordinate, value);
    }
    m_kd_tree_shifts->build_tree_structure();
};

bool LocalShiftsHandler::calculate_shifted_coordinates(int x, int y, int *shifted_x, int *shifted_y) const  {
    const int n_neighbors = 3;
    const int query_point[2] = {x, y};
    std::vector<std::tuple<std::vector<int>, tuple<int,int,bool>>> closest_nodes = m_kd_tree_shifts->get_k_nearest_neighbors(query_point, n_neighbors);
    if (closest_nodes.size() == 0) {
        return false;
    }
    if (get<2>(get<1>(closest_nodes[0])) == false) {
        return false;
    }
    float sum_weights = 0;
    float sum_x = 0;
    float sum_y = 0;
    for (const std::tuple<std::vector<int>, tuple<int,int,bool>> &node: closest_nodes) {
        const vector<int> &coordinates = get<0>(node);
        const tuple<int,int,bool> &value = get<1>(node);
        const int dx = get<0>(value);
        const int dy = get<1>(value);
        const bool valid_ap = get<2>(value);

        if (!valid_ap) {
            continue;
        }

        const float dist2 = (x - coordinates[0]) * (x - coordinates[0]) + (y - coordinates[1]) * (y - coordinates[1]);
        if (dist2 == 0) {
            *shifted_x = coordinates[0];
            *shifted_y = coordinates[1];
            return true;
        }
        const float weight = 1.0 / dist2;
        sum_weights += weight;
        sum_x += weight * dx;
        sum_y += weight * dy;
    }
    *shifted_x = sum_x / sum_weights;
    *shifted_y = sum_y / sum_weights;
    return true;
};

