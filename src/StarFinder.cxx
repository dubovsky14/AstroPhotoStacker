#include "../headers/StarFinder.h"

using namespace std;
using namespace AstroPhotoStacker;

void AstroPhotoStacker::sort_stars_by_size(std::vector<std::tuple<float, float,int> > *stars) {
    std::sort(stars->begin(), stars->end(), [](const std::tuple<float, float,int> &a, const std::tuple<float, float,int> &b) {
        return std::get<2>(a) > std::get<2>(b);
    });
};

void AstroPhotoStacker::keep_only_stars_above_size(std::vector<std::tuple<float, float,int> > *stars, int min_size) {
    stars->erase(std::remove_if(stars->begin(), stars->end(), [min_size](const std::tuple<float, float,int> &star) {
        return std::get<2>(star) < min_size;
    }), stars->end());
};


std::tuple<float,float> AstroPhotoStacker::get_center_of_cluster(const std::vector<std::tuple<int,int>>  &cluster)   {
    float x_sum(0), y_sum(0);
    for (const std::tuple<int,int> &pixel : cluster)  {
        x_sum += std::get<0>(pixel);
        y_sum += std::get<1>(pixel);
    }
    return std::make_tuple(x_sum / cluster.size(), y_sum / cluster.size());
};

bool AstroPhotoStacker::is_border_pixel(const std::tuple<int,int> &pixel, const std::vector<std::tuple<int,int> > &cluster) {
    for (int shift_x = -1; shift_x <= 1; shift_x++) {
        for (int shift_y = -1; shift_y <= 1; shift_y++) {
            if (shift_x == 0 && shift_y == 0) continue;
            const std::tuple<int,int> pixel_to_check(std::get<0>(pixel) + shift_x, std::get<1>(pixel) + shift_y);
            if (std::find(cluster.begin(), cluster.end(), pixel_to_check) == cluster.end()) {
                return true;
            }
        }
    }
    return false;
};

vector<tuple<float,float>> AstroPhotoStacker::get_cluster_smearing_vector(const vector< vector<tuple<int, int> > > &clusters)    {
    using namespace MyTupleArithmetics;

    vector<tuple<float,float>> result;
    for (const auto &cluster : clusters)    {
        const tuple<float,float> center_of_cluster = get_center_of_cluster(cluster);
        float max_radius_size(0), min_radius_size(10e10);
        tuple<float,float> max_radius, min_radius;
        for (const auto &pixel : cluster)   {
            if (is_border_pixel(pixel, cluster)) {
                const tuple<float,float> pos_wrt_center = pixel - center_of_cluster;
                const float distance_squared = get_distance_squared(pos_wrt_center);
                if (distance_squared > max_radius_size) {
                    max_radius_size = distance_squared;
                    max_radius = pos_wrt_center;
                }
                if (distance_squared < min_radius_size) {
                    min_radius_size = distance_squared;
                    min_radius = pos_wrt_center;
                }
            }
        }
        max_radius_size = sqrt(max_radius_size);
        min_radius_size = sqrt(min_radius_size);
        const float shift_size = max_radius_size - min_radius_size;
        const float max_radius_sin = std::get<1>(max_radius) / max_radius_size;
        const float max_radius_phi = asin(max_radius_sin);

        result.push_back(tuple<float,float>(shift_size, max_radius_phi));
    }
    return result;
};

float AstroPhotoStacker::get_distance_squared(const tuple<float,float> &vec)   {
    return std::get<0>(vec)*std::get<0>(vec) + std::get<1>(vec)*std::get<1>(vec);
};