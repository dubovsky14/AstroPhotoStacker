#pragma once

#include "../headers/LocalShift.h"
#include "../headers/KDTree.h"

#include <vector>


namespace AstroPhotoStacker {

    /**
     * @brief Tool for clustering local shifts based on their proximity. The goal is to eliminate redundant local shifts that are very close to each other and are causing problems in KNN searches later.
     */
    class LocalShiftsClusteringTool {
        public:
            LocalShiftsClusteringTool() = delete;

            LocalShiftsClusteringTool(float cluster_radius_in_pixels)  { m_cluster_radius_in_pixels = cluster_radius_in_pixels;  };

            /**
             * @brief Cluster the local shifts based on their proximity. Only one shift per cluster is returned with values averaged over the cluster.
             *
             * @param local_shifts - input local shifts
             * @return std::vector<LocalShift> - clustered local shifts
            */
            std::vector<LocalShift> cluster_local_shifts(const std::vector<LocalShift> &local_shifts) const;

        private:
            float m_cluster_radius_in_pixels = 10.0;

            void build_cluster_from_point(const KDTree<float,2, int> &kd_tree, const std::vector<LocalShift> &local_shifts, std::vector<int> *clustered_indices, int point_index, int cluster_index)    const;

    };
}