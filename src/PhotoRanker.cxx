#include "../headers/PhotoRanker.h"
#include "../headers/StarFinder.h"
#include "../headers/raw_file_reader.h"

#include <filesystem>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;


PhotoRanker::PhotoRanker(const std::vector<std::string> &path_to_input_files)   {
    m_input_files = path_to_input_files;
};

PhotoRanker::PhotoRanker(const std::string& path_to_lights_folder)  {
    for (const auto & entry : filesystem::directory_iterator(path_to_lights_folder)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        m_input_files.push_back(entry.path());
    }
};

void PhotoRanker::rank_all_files()    {
    for (const auto &file : m_input_files) {
        const float ranking = calculate_file_ranking(file);
        m_ranking.push_back(ranking);
    }
};

float PhotoRanker::calculate_file_ranking(const std::string &file_address)  {
    int width, height;
    std::unique_ptr<unsigned short[]> brightness = read_raw_file(file_address, &width, &height);
    const float threshold_value = get_threshold_value(brightness.get(), width*height, 0.002);
    std::vector<std::vector<std::tuple<int,int>>> clusters = get_clusters(brightness.get(), width, height, threshold_value);

    vector<float> cluster_excentricities;
    for (const auto &cluster : clusters) {
        if (cluster.size() < 20)    continue;
        cluster_excentricities.push_back(get_cluster_excentricity(cluster));
    }

    if (cluster_excentricities.size() == 0) {
        throw std::runtime_error("No clusters found in file " + file_address);
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


std::vector<std::tuple<std::string,float>> PhotoRanker::get_ranking() const {
    std::vector<std::tuple<std::string,float>> result;
    for (unsigned int i = 0; i < m_input_files.size(); i++) {
        result.push_back(std::make_tuple(m_input_files[i], m_ranking[i]));
    }
    sort(result.begin(), result.end(), [](const std::tuple<std::string,float> &a, const std::tuple<std::string,float> &b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    return result;
};