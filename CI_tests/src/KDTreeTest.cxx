#include "../headers/KDTreeTest.h"

#include "../../headers/KDTreeWithBuffer.h"
#include "../../headers/Common.h"

#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <algorithm>

#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

TestResult AstroPhotoStacker::test_kd_tree()   {
    vector<vector<float> > random_points;
    for (int i = 0; i < 10000; i++) {
        random_points.push_back({random_uniform(-2,6), random_uniform(0, 10), random_uniform(-1,1)});
    }

    KDTreeWithBuffer<float, 3, int> tree;
    for (size_t i = 0; i < random_points.size(); i++) {
        tree.add_point(random_points[i], i);
    }
    tree.build_tree_structure();

    float query_point[3] = {2, 5, 0};

    unsigned int n_neighbors = 10;
    const vector<std::tuple<std::array<float, 3>, int>> nearest_neighbors = tree.get_k_nearest_neighbors(query_point, n_neighbors);

    for (const auto &node : nearest_neighbors) {
        float distance2 = 0;
        for (int index = 0; index < 3; index++) {
            distance2 += (get<0>(node)[index] - query_point[index])*(get<0>(node)[index] - query_point[index]);
        }
    }

    vector<tuple<int, float>> index_distance2_vector;
    for (size_t i = 0; i < random_points.size(); i++) {
        const vector<float> &point = random_points[i];
        float distance2 = 0;
        for (int index = 0; index < 3; index++) {
            distance2 += (point[index] - query_point[index])*(point[index] - query_point[index]);
        }
        index_distance2_vector.push_back({i, distance2});
    }

    sort(index_distance2_vector.begin(), index_distance2_vector.end(), [](const tuple<int, float> &a, const tuple<int, float> &b) {
        return get<1>(a) < get<1>(b);
    });


    for (unsigned int i = 0; i < n_neighbors; i++) {
        if (get<1>(nearest_neighbors[i]) != get<0>(index_distance2_vector[i])) {
            return TestResult(false, "KDTree test failed. Brute force and KDTree results do not match.");
        }
    }
    return TestResult(true, "");
}