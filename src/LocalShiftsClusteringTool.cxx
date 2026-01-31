#include "../headers/LocalShiftsClusteringTool.h"


#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


std::vector<LocalShift> LocalShiftsClusteringTool::cluster_local_shifts(const std::vector<LocalShift> &local_shifts) const  {
    std::vector<LocalShift> valid_local_shifts;
    for (const LocalShift &shift : local_shifts) {
        if (shift.valid_ap) {
            valid_local_shifts.push_back(shift);
        }
    }


    if (valid_local_shifts.size() == 0) {
        return std::vector<LocalShift>();
    }

    vector<int> clustered_indices(valid_local_shifts.size(), -1);

    KDTree<float,2, int> kd_tree;
    for (unsigned int i = 0; i < valid_local_shifts.size(); i++) {
        const LocalShift &shift = valid_local_shifts[i];
        kd_tree.add_point( {float(shift.x), float(shift.y)}, i);
    }
    kd_tree.build_tree_structure();

    int current_cluster_index = 0;
    for (unsigned int i = 0; i < valid_local_shifts.size(); i++) {
        if (clustered_indices[i] != -1) {
            continue;
        }
        build_cluster_from_point( kd_tree, valid_local_shifts, &clustered_indices, i, current_cluster_index);
        current_cluster_index++;
    };

    struct ClusterShiftsData {
        int n_points = 0;
        double sum_x = 0.0f;
        double sum_y = 0.0f;
        double sum_dx = 0.0f;
        double sum_dy = 0.0f;
        double sum_score = 0.0f;
    };

    vector<ClusterShiftsData> clusters_data(current_cluster_index);
    for (unsigned int i = 0; i < valid_local_shifts.size(); i++) {
        const int cluster_index = clustered_indices[i];
        ClusterShiftsData &cluster_data = clusters_data[cluster_index];
        const LocalShift &shift = valid_local_shifts[i];
        cluster_data.n_points++;
        cluster_data.sum_x += shift.x;
        cluster_data.sum_y += shift.y;
        cluster_data.sum_dx += shift.dx;
        cluster_data.sum_dy += shift.dy;
        cluster_data.sum_score += shift.score;
    }

    vector<LocalShift> clustered_local_shifts;
    for (const ClusterShiftsData &cluster_data : clusters_data) {
        LocalShift averaged_shift;
        averaged_shift.x = int(cluster_data.sum_x / cluster_data.n_points + 0.5);
        averaged_shift.y = int(cluster_data.sum_y / cluster_data.n_points + 0.5);
        averaged_shift.dx = int(cluster_data.sum_dx / cluster_data.n_points + 0.5);
        averaged_shift.dy = int(cluster_data.sum_dy / cluster_data.n_points + 0.5);
        averaged_shift.valid_ap = true;
        averaged_shift.score = float(cluster_data.sum_score / cluster_data.n_points);
        clustered_local_shifts.push_back(averaged_shift);
    }

    return clustered_local_shifts;
};

void LocalShiftsClusteringTool::build_cluster_from_point(const KDTree<float,2, int> &kd_tree, const std::vector<LocalShift> &local_shifts, std::vector<int> *clustered_indices, int point_index, int cluster_index) const {
    const vector<long long int> neighbor_indices = kd_tree.get_nodes_closer_than_x( kd_tree.get_node(point_index).m_coordinates.data(), m_cluster_radius_in_pixels);
    struct NeighborInfo {
        int             local_shift_index;
        long long int   kd_tree_node_index;
        unsigned int    unclustered_neighbors;
    };
    vector<NeighborInfo> unclustered_neighbors; // pair<local_shift_index, kd-tree node index>
    for (long long int neighbor_node_index : neighbor_indices) {
        const KDTreeNode<float, 2, int> &this_node =  kd_tree.get_node(neighbor_node_index);
        const int local_shift_index = this_node.m_value;
        if (local_shift_index == point_index) {
            continue;
        }
        if ( (*clustered_indices)[local_shift_index] != -1) {
            continue;
        }

        const vector<long long int> neighbors_of_this_neighbor = kd_tree.get_nodes_closer_than_x( this_node.m_coordinates.data(), m_cluster_radius_in_pixels);
        unsigned int n_unclustered_neighbors = 0;
        for (long long int neighbor_of_neighbor_node_index : neighbors_of_this_neighbor) {
            const KDTreeNode<float, 2, int> &neighbor_of_this__neighbor =  kd_tree.get_node(neighbor_of_neighbor_node_index);
            const int neighbor_local_shift_index = neighbor_of_this__neighbor.m_value;
            if (neighbor_local_shift_index == local_shift_index || neighbor_local_shift_index == point_index) { // skip self and the original point
                continue;
            }
            if ( (*clustered_indices)[neighbor_local_shift_index] == -1) {
                n_unclustered_neighbors++;
            }
        }
        unclustered_neighbors.push_back( NeighborInfo{ local_shift_index, neighbor_node_index, n_unclustered_neighbors } );
    }
    if (unclustered_neighbors.size() == 0) {
        clustered_indices->at(point_index) = cluster_index;
        return;
    }

    sort( unclustered_neighbors.begin(), unclustered_neighbors.end(), [](const NeighborInfo &a, const NeighborInfo &b) {
        return a.unclustered_neighbors > b.unclustered_neighbors;
    } );

    if (unclustered_neighbors.at(0).unclustered_neighbors <= unclustered_neighbors.size()) {
        for (const NeighborInfo &neighbor_info : unclustered_neighbors) {
            clustered_indices->at( neighbor_info.local_shift_index ) = cluster_index;
        }
        clustered_indices->at(point_index) = cluster_index;
        return;
    }

    build_cluster_from_point( kd_tree, local_shifts, clustered_indices, unclustered_neighbors.at(0).local_shift_index, cluster_index);
}