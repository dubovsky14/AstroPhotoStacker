#include "../headers/StarFinder.h"

using namespace std;

void sort_stars_by_size(std::vector<std::tuple<float, float,int> > *stars) {
    std::sort(stars->begin(), stars->end(), [](const std::tuple<float, float,int> &a, const std::tuple<float, float,int> &b) {
        return std::get<2>(a) > std::get<2>(b);
    });
};

void keep_only_stars_above_size(std::vector<std::tuple<float, float,int> > *stars, int min_size) {
    stars->erase(std::remove_if(stars->begin(), stars->end(), [min_size](const std::tuple<float, float,int> &star) {
        return std::get<2>(star) < min_size;
    }), stars->end());
};