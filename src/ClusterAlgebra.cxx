#include "../headers/ClusterAlgebra.h"
#include "../headers/StarFinder.h"
#include "../headers/InputFrameReader.h"
#include "../headers/Common.h"
#include "../headers/VideoReader.h"

#include <filesystem>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;


float AstroPhotoStacker::ClusterAlgebra::get_cluster_excentricity(const std::vector<std::tuple<int,int>> &cluster)   {
    using namespace MyTupleArithmetics;
    const std::tuple<float,float> center_of_cluster = get_center_of_cluster(cluster);
    float max_radius_squared(0);
    for (const auto &pixel : cluster)   {
        const float distance_from_center_squared = get_distance_squared(center_of_cluster-pixel);
        if (distance_from_center_squared > max_radius_squared) {
            max_radius_squared = distance_from_center_squared;
        }
    }
    const float radius_ideal_circle = sqrt(cluster.size() / M_PI);
    const float max_radius = sqrt(max_radius_squared);
    return abs(max_radius - radius_ideal_circle);
};

float AstroPhotoStacker::ClusterAlgebra::get_cluster_correlation(const std::vector<std::tuple<int,int>> &cluster) {
    const std::vector<std::vector<float>> covariance_matrix = get_covariance_matrix(cluster);
    return covariance_matrix[0][1]/(sqrt(covariance_matrix[0][0]*covariance_matrix[1][1]));
};

std::vector<std::vector<float>> AstroPhotoStacker::ClusterAlgebra::get_covariance_matrix(const std::vector<std::tuple<int,int>> &cluster)    {
    using namespace MyTupleArithmetics;
    const std::tuple<float,float> center_of_cluster = get_center_of_cluster(cluster);
    const std::tuple<double,double> center_of_cluster_double = {get<0>(center_of_cluster),get<1>(center_of_cluster)};
    double sum_xy(0), sum_x_squared(0), sum_y_squared(0);
    for (const auto &pixel : cluster)   {
        const tuple<double,double> pixel_relative_to_center = pixel - center_of_cluster_double;
        const double x = std::get<0>(pixel_relative_to_center);
        const double y = std::get<1>(pixel_relative_to_center);
        sum_xy += x * y;
        sum_x_squared += x * x;
        sum_y_squared += y * y;
    }
    sum_xy /= cluster.size();
    sum_x_squared /= cluster.size();
    sum_y_squared /= cluster.size();

    return {{float(sum_x_squared), float(sum_xy)},{float(sum_xy), float(sum_y_squared)}};
};

float AstroPhotoStacker::ClusterAlgebra::get_covariance_eigenvalues_ratio_sqrt(const std::vector<std::tuple<int,int>> &cluster)    {
    const std::vector<std::vector<float>> covariance_matrix = get_covariance_matrix(cluster);
    vector<float> eigenvalues;
    vector<vector<float>> eigenvectors;
    const bool eigenvals_valid = calculate_eigenvectors_and_eigenvalues(covariance_matrix, &eigenvalues, &eigenvectors);

    if (eigenvalues.size() != 2)    return 0;
    if (!eigenvals_valid)           return 0;
    const float eigen_larger  = eigenvalues[0] > eigenvalues[1] ? eigenvalues[0] : eigenvalues[1];
    const float eigen_smaller = eigenvalues[0] > eigenvalues[1] ? eigenvalues[1] : eigenvalues[0];

    if (eigen_smaller == 0) return 1e10;
    const float ratio = eigen_larger/eigen_smaller;
    if (ratio < 0)  return 0;
    return sqrt(ratio);
};
