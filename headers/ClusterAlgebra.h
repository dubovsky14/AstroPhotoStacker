#pragma once

#include "../headers/InputFrame.h"

#include <string>
#include <vector>
#include <array>
namespace AstroPhotoStacker     {

    /**
     * @brief Class to rank photos based on the tracking performance of the mount
    */
    namespace ClusterAlgebra   {
            /**
             * @brief Calculate excentricity of a cluster, defined as distance between 2 most distant points minus diameter of a circle with the same area
             *
             * @param clusters - stars
             * @return float - excentricity
            */
           float get_cluster_excentricity(const std::vector<std::tuple<int,int>> &cluster);

            /**
             * @brief Calculate correlation between x and y coordinates of a cluster
             *
             * @param clusters - stars
             * @return float - correlation
            */
           float get_cluster_correlation(const std::vector<std::tuple<int,int>> &cluster);

            /**
             * @brief Calculate correlation between x and y coordinates of a cluster
             *
             * @param clusters - stars
             * @return float - correlation
            */
           std::vector<std::vector<float>> get_covariance_matrix(const std::vector<std::tuple<int,int>> &cluster);

            /**
             * @brief calculate eigen values of the covariance matrix of the cluster coordinates and divide larger one by the smaller one (should be large number for lines, or elongated objects)
             *
             * @param clusters
             * @return float - ratio of the eigenvalues
             */
           float get_covariance_eigenvalues_ratio_sqrt(const std::vector<std::tuple<int,int>> &cluster);

    };
}