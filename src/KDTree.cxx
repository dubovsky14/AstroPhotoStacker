#include "../headers/KDTree.h"
#include "../headers/Common.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <map>
#include <filesystem>
#include <cstring>

using namespace AstroPhotoStacker;
using namespace std;

PointInKDTree::PointInKDTree()  {
    m_coordinates[0] = 0;
    m_coordinates[1] = 0;
    m_coordinates[2] = 0;
    m_coordinates[3] = 0;
    m_star_indices = StarIndices(0,0,0,0);
};

PointInKDTree::PointInKDTree(const PointCoordinatesTuple &coordinates, const StarIndices &star_indices)   {
    m_coordinates[0]    = get<0>(coordinates);
    m_coordinates[1]    = get<1>(coordinates);
    m_coordinates[2]    = get<2>(coordinates);
    m_coordinates[3]    = get<3>(coordinates);
    m_star_indices      = star_indices;
};


PointIndexType PointInKDTree::get_child_index(const CoordinateDataType *coordinates) {
    if (m_index_for_splitting < 0)  {
        return -1;
    }

    if (coordinates[m_index_for_splitting] > m_coordinates[m_index_for_splitting])  {
        return m_index_child_right;
    }
    return m_index_child_left;
};

KDTree::KDTree(unsigned int n_points)   {
    m_points_in_tree.reserve(n_points);
};

void KDTree::add_point(const PointCoordinatesTuple &coordinates, const StarIndices &star_indices) {
    m_points_in_tree.push_back(PointInKDTree(coordinates, star_indices));
    m_number_of_points_in_tree++;
};

void KDTree::create_tree_structure()    {
    vector<unsigned int> all_indices(m_points_in_tree.size());
    for (unsigned int i = 0; i< m_points_in_tree.size(); i++)   {
        all_indices[i] = i;
    }
    m_root_node_index = build_node(&all_indices, -1);
};

vector<std::tuple<PointCoordinatesTuple, StarIndices> > KDTree::get_k_nearest_neighbors(const PointCoordinatesTuple &query_point, unsigned int n_points)    const   {
    std::vector<unsigned int> indices = get_k_nearest_neighbors_indices(query_point, n_points);
    vector<std::tuple<PointCoordinatesTuple, StarIndices> > result;
    result.reserve(indices.size());
    for (unsigned int index : indices)  {
        const PointInKDTree point = get_point_in_tree(index);
        result.push_back (
            tuple<PointCoordinatesTuple, StarIndices>   (
                PointCoordinatesTuple(  point.m_coordinates[0],
                                        point.m_coordinates[1],
                                        point.m_coordinates[2],
                                        point.m_coordinates[3]),
                point.m_star_indices
            )
        );
    }
    return result;
};


std::vector<unsigned int> KDTree::get_k_nearest_neighbors_indices(const PointCoordinatesTuple &query_point, unsigned int n_points)  const  {
    vector<tuple <unsigned int, float> > vector_index_distance; // indices of n_points nearest neighbors and their distances
    vector_index_distance.reserve(n_points);
    map<unsigned int, char> visited_nodes;

    CoordinateDataType query_point_array[4];
    query_point_array[0]    = get<0>(query_point);
    query_point_array[1]    = get<1>(query_point);
    query_point_array[2]    = get<2>(query_point);
    query_point_array[3]    = get<3>(query_point);

    if (m_root_node_index < 0)  {
        return vector<unsigned int>();
    }


    int node_index = m_root_node_index;
    int leaf_node_index = m_root_node_index;
    // find the leaf node
    while   (node_index >= 0)   {
        visited_nodes[node_index] = 1;
        const float distance = get_distance(query_point_array, get_point_in_tree(node_index).m_coordinates);
        add_node_to_vector_index_distance(distance, &vector_index_distance, n_points, node_index);

        leaf_node_index = node_index;
        node_index = get_point_in_tree(node_index).get_child_index(query_point_array);
    }

    // now go upwards the tree
    node_index = leaf_node_index;
    while   (node_index >= 0)   {
        scan_children_nodes(node_index, &visited_nodes, &vector_index_distance, query_point_array);
        node_index = get_point_in_tree(node_index).m_index_parent;
    }

    vector<unsigned int>    result;
    result.reserve(vector_index_distance.size());
    for (const tuple <unsigned int, float> &node : vector_index_distance)   {
        result.push_back(get<0>(node));
    }
    return result;
};


int KDTree::build_node(std::vector<unsigned int> *sub_indices, int parent_index) {
    const bool has_parent = parent_index >= 0;
    short index_for_splitting = has_parent ? (m_points_in_tree[parent_index].m_index_for_splitting + 1) % 4 : 0;

    unsigned int this_point_index;
    const CoordinateDataType median = get_median_and_its_index_from_sample(*sub_indices, index_for_splitting, &this_point_index);

    PointInKDTree &this_point = m_points_in_tree[this_point_index];

    this_point.m_index_for_splitting    = index_for_splitting;
    this_point.m_index_parent           = parent_index;

    vector<unsigned int> indices_child_left;
    vector<unsigned int> indices_child_right;
    indices_child_left.reserve(sub_indices->size()/2);
    indices_child_right.reserve(sub_indices->size()/2);
    split_based_on_cut(*sub_indices, index_for_splitting, median, this_point_index, &indices_child_left, &indices_child_right);

    sub_indices->clear(); // otherwise it would eat up memory
    if (indices_child_left.size())  {
        this_point.m_index_child_left = build_node(&indices_child_left, this_point_index);
    }
    if (indices_child_right.size()) {
        this_point.m_index_child_right = build_node(&indices_child_right, this_point_index);
    }


    return this_point_index;
};

void KDTree::split_based_on_cut(const vector<unsigned int> &sub_indices, short coordinate_index,
                                CoordinateDataType cut_value, unsigned int index_to_omit,
                                vector<unsigned int> *indices_lower_value, vector<unsigned int> *indices_higher_value)  {
    for (unsigned int sub_index : sub_indices)  {
        if (sub_index == index_to_omit) {
            continue;
        }
        const PointInKDTree &this_point = get_point_in_tree(sub_index);
        const CoordinateDataType value_this_point = this_point.m_coordinates[coordinate_index];
        if (value_this_point > cut_value)   indices_higher_value->push_back(sub_index);
        else                                indices_lower_value->push_back(sub_index);
    }
};

CoordinateDataType KDTree::get_median_and_its_index_from_sample(const std::vector<unsigned int> &sub_indices, short coordinate, unsigned int *median_index)   const   {
    // yes, this function is a mess, I know. But if you want to use processor cache and not to jump in the memory by few hundred MBs in each sort comparison, you don't have too much of a choice ...

    vector<unsigned int> subsample_indices;
    vector<unsigned int> indices_in_subsample_vector;
    if (sub_indices.size() < 200)   {
        subsample_indices = sub_indices;
    }
    else {
        for (unsigned int i = 0; i < 200; i++)  {
            unsigned int random_index = random_uniform()*sub_indices.size();
            if (random_index == sub_indices.size()) random_index--;
            subsample_indices.push_back(sub_indices.at(random_index));
        }
    }
    indices_in_subsample_vector.resize(subsample_indices.size());

    // you definitely do not want to grab value from points_in_tree during sorting and jump randomly over hundreds MBs of memory ...
    vector<CoordinateDataType> subsample_values(subsample_indices.size());
    for (unsigned int i = 0; i < subsample_indices.size(); i++)  {
        indices_in_subsample_vector.at(i) = i;
        subsample_values.at(i) = get_point_in_tree( subsample_indices.at(i) ).m_coordinates[coordinate];
    }

    sort(indices_in_subsample_vector.begin(), indices_in_subsample_vector.end(), [subsample_values](unsigned int a, unsigned int b){
        return subsample_values.at(a) > subsample_values.at(b);
    });

    vector<unsigned int> subsample_indices_sorted(subsample_indices.size());
    for (unsigned int i = 0; i < indices_in_subsample_vector.size(); i++)  {
        subsample_indices_sorted.at(i) = sub_indices.at(indices_in_subsample_vector.at(i));
    }

    const unsigned int middle_index = subsample_indices_sorted.at(subsample_indices_sorted.size()/2);
    if (median_index) *median_index = middle_index;

    return get_point_in_tree(middle_index).m_coordinates[coordinate];
};


float KDTree::get_distance(const CoordinateDataType *x1, const CoordinateDataType *x2)    {
    float distance2 = pow2(x1[0]-x2[0]) + pow2(x1[1]-x2[1]) + pow2(x1[2]-x2[2]) + pow2(x1[3]-x2[3]);
    return sqrt(distance2);
};


void KDTree::scan_children_nodes (   unsigned int node_index,
                                        std::map<unsigned int, char> *visited_nodes,
                                        std::vector<std::tuple <unsigned int, float> > *vector_index_distance,
                                        const CoordinateDataType *query_point_array
                                        )   const   {
    if (node_index < 0)   {
        return;
    }

    if (visited_nodes->find(node_index) != visited_nodes->end())  {
        return;
    }
    (*visited_nodes)[node_index] = 1;

    const short split_index = get_point_in_tree(node_index).m_index_for_splitting;
    const float dist_from_splitting_plane = fabs(query_point_array[split_index] - get_point_in_tree(node_index).m_coordinates[split_index]);

    if (dist_from_splitting_plane >= get<1>(vector_index_distance->back())) {
        return;
    }

    const bool query_point_on_right = query_point_array[split_index] > get_point_in_tree(node_index).m_coordinates[split_index];
    if (query_point_on_right)   {
        const int child_id = get_point_in_tree(node_index).m_index_child_right;
        scan_children_nodes(child_id, visited_nodes, vector_index_distance, query_point_array);
    }
    else   {
        const int child_id = get_point_in_tree(node_index).m_index_child_left;
        scan_children_nodes(child_id, visited_nodes, vector_index_distance, query_point_array);
    }
};



void KDTree::add_node_to_vector_index_distance(float distance, vector<tuple <unsigned int, float> > *vector_index_distance, unsigned int n_points, unsigned int node_index)   {
    if (vector_index_distance->size() < n_points)    {
        vector_index_distance->push_back(tuple<unsigned int, float>(node_index,distance));
    }
    else if (get<1>(vector_index_distance->at(n_points-1)) > distance)  {
        vector_index_distance->at(n_points-1) = tuple<unsigned int, float>(node_index,distance);
    }

    sort(vector_index_distance->begin(), vector_index_distance->end(), [](const auto &a, const auto &b){
        return get<1>(a) < get<1>(b);
    });
};

void KDTree::get_point(unsigned int node_index, PointCoordinatesArray coordinates, StarIndices *star_indices) const    {
    const PointInKDTree &point_in_tree = get_point_in_tree(node_index);
    memcpy(coordinates, point_in_tree.m_coordinates, sizeof(point_in_tree.m_coordinates));
    *star_indices = point_in_tree.m_star_indices;
};

PointInKDTree KDTree::get_point_in_tree(unsigned int node_index) const   {
    return m_points_in_tree[node_index];
};
