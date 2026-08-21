#include "../headers/PhotoRanker.h"
#include "../headers/StarFinder.h"
#include "../headers/InputFrameReader.h"
#include "../headers/Common.h"
#include "../headers/VideoReader.h"

#include <filesystem>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;


PhotoRanker::PhotoRanker(const std::vector<InputFrame> &input_frames)   {
    m_input_frames = input_frames;
};

PhotoRanker::PhotoRanker(const std::string& path_to_lights_folder)  {
    const vector<string> files = get_frame_files_in_folder(path_to_lights_folder);
    for (const string &file : files) {
        if (is_valid_video_file(file))  {
            const int number_of_frames = get_number_of_frames_in_video(file);
            for (int i = 0; i < number_of_frames; i++) {
                m_input_frames.push_back(InputFrame(file, i));
            }
            continue;
        }
        else {
            m_input_frames.push_back(InputFrame(file));
        }
    }
};

void PhotoRanker::rank_all_files()    {
    for (const InputFrame &frame : m_input_frames) {
        const float ranking = calculate_frame_ranking(frame);
        m_ranking.push_back(ranking);
    }
};

float PhotoRanker::calculate_frame_ranking(const InputFrame &input_frame)  {
    int width, height;
    InputFrameReader input_frame_reader(input_frame);
    const std::vector<PixelType> brightness = input_frame_reader.get_monochrome_data();
    input_frame_reader.get_photo_resolution(&width, &height);

    const float threshold_value = get_threshold_value(brightness.data(), width*height, 0.002);
    std::vector<std::vector<std::tuple<int,int>>> clusters = get_clusters(brightness.data(), width, height, threshold_value);

    vector<float> cluster_excentricities;
    for (const auto &cluster : clusters) {
        if (cluster.size() < 20)    continue;
        cluster_excentricities.push_back(get_cluster_excentricity(cluster));
    }

    if (cluster_excentricities.size() == 0) {
        throw std::runtime_error("No clusters found in frame: " + input_frame.to_string());
    }

    sort(cluster_excentricities.begin(), cluster_excentricities.end());
    return cluster_excentricities[cluster_excentricities.size() / 5];
};

float PhotoRanker::get_cluster_excentricity(const std::vector<std::tuple<int,int>> &cluster)   {
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

float PhotoRanker::get_cluster_correlation(const std::vector<std::tuple<int,int>> &cluster) {
    const std::vector<std::vector<float>> covariance_matrix = get_covariance_matrix(cluster);
    return covariance_matrix[0][1]/(sqrt(covariance_matrix[0][0]*covariance_matrix[1][1]));
};

std::vector<std::vector<float>> PhotoRanker::get_covariance_matrix(const std::vector<std::tuple<int,int>> &cluster)    {
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

float PhotoRanker::get_covariance_eigenvalues_ratio_sqrt(const std::vector<std::tuple<int,int>> &cluster)    {
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

std::vector<std::tuple<InputFrame,float>> PhotoRanker::get_ranking() const {
    std::vector<std::tuple<InputFrame,float>> result;
    for (unsigned int i = 0; i < m_input_frames.size(); i++) {
        result.push_back(std::make_tuple(m_input_frames[i], m_ranking[i]));
    }
    sort(result.begin(), result.end(), [](const std::tuple<InputFrame,float> &a, const std::tuple<InputFrame,float> &b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    return result;
};
