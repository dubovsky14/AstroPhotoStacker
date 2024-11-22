#pragma once

#include "../headers/Common.h"

#include<tuple>
#include<vector>
#include<map>
#include<string>
#include<memory>
#include<algorithm>
#include<fstream>
#include <stdlib.h>
#include <array>

namespace AstroPhotoStacker   {
    typedef int PointIndexType;

    template<typename CoordinateType, unsigned int NumberOfCoordinates, typename ValueType>
    class KDTreeNode {
        public:
            KDTreeNode() {
                for (unsigned int i = 0; i < NumberOfCoordinates; ++i)  {
                    m_coordinates[i] = 0;
                }
            };
            KDTreeNode(const CoordinateType *coordinates, const ValueType &star_indices);

            PointIndexType get_child_index(const CoordinateType *coordinates);

            std::array<CoordinateType, NumberOfCoordinates> m_coordinates;
            ValueType           m_value;
            short               m_split_axis            = -1;
            PointIndexType      m_index_child_left      = -1;
            PointIndexType      m_index_child_right     = -1;
            PointIndexType      m_index_parent          = -1;

    };

    /**
     * @brief Class implementing k-dimensional binary tree, which can be used to search for the nearest neighbors of a point
     *
     */
    template<typename CoordinateType, unsigned int NumberOfCoordinates, typename ValueType>
    class KDTree    {
        public:
            KDTree()  {
                m_n_dim = NumberOfCoordinates;
            };

            KDTree(const KDTree<CoordinateType, NumberOfCoordinates, ValueType> &kd_tree) = default;

            /**
             * @brief Get const reference to the node object, given index node_index
             *
             * @param node_index
             * @return const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType>&
             */
            const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &get_node(const long long int node_index) const {
                return m_nodes[node_index];
            };


            /**
             * @brief Get the number of nodes in the tree
             *
             * @return long long int
             */
            long long int get_n_nodes() const {
                return m_nodes.size();
            };

            /**
             * @brief Get index of the root node
             *
             */
            long long int get_root_node_index() const {
                return m_root_node_index;
            }

            /**
             * @brief Get const reference to the vector of nodes
             *
             * @return const std::vector<KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType>> &
             */
            const std::vector<KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType>> &get_nodes() const {
                return m_nodes;
            };


            /**
             * @brief Add point (node) to the tree. The tree structure must not be built yet.
             *
             * @param coordinates
             * @param value - these are the additional data associated with the node
             */
            void add_point(const std::vector<CoordinateType> &coordinates, const ValueType &value)    {
                if (m_tree_structure_built) {
                    throw std::runtime_error("KDTree::add_point: tree structure already built.");
                }

                if (coordinates.size() != m_n_dim) {
                    throw std::runtime_error("KDTree::add_point: coordinates must have m_n_dim elements.");
                }
                add_point(coordinates.data(), value);
            };


            /**
             * @brief Add point (node) to the tree. The tree structure must not be built yet.
             *
             * @param coordinates
             * @param value - these are the additional data associated with the node
             */
            void add_point(const CoordinateType *coordinates, const ValueType &value)   {
                KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> node;
                for (unsigned int i = 0; i < m_n_dim; ++i)  {
                    node.m_coordinates[i] = coordinates[i];
                }
                node.m_value = value;

                m_nodes.push_back(node);
            }

            /**
             * @brief Add points (nodes) to the tree. The tree structure must not be built yet.
             *
             * @param coordinates - vector of coordinates
             * @param values - vector of the additional data associated with the nodes
             */
            void add_points(const std::vector<std::vector<CoordinateType>> &coordinates, const std::vector<ValueType> &values)    {
                if (coordinates.size() != values.size())    {
                    throw std::runtime_error("KDTree::add_points: coordinates and values must have the same size.");
                }
                for (const auto &coordinate : coordinates)  {
                    if (coordinate.size() != m_n_dim)  {
                        throw std::runtime_error("KDTree::add_points: coordinates must have m_n_dim elements.");
                    }
                }

                for (int i = 0; i < coordinates.size(); ++i)  {
                    add_point(coordinates[i], values[i]);
                }
            };

            std::vector<std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>> get_k_nearest_neighbors(const CoordinateType *query_point, unsigned int n_points)    const   {
                std::vector<long long int> indices = get_k_nearest_neighbors_indices(query_point, n_points);
                std::vector<std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>> result;
                result.reserve(indices.size());
                for (long long int index : indices)  {
                    const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &point = m_nodes[index];
                    std::array<CoordinateType, NumberOfCoordinates> coordinates = point.m_coordinates;
                    result.push_back (
                        std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>   (
                            coordinates,
                            point.m_value
                        )
                    );
                }
                return result;
            };

            /**
             * @brief Get vector of indices of n closest points to the query point.
             *
             * @param coordinates of the query point
             * @param n - number of closest points to return
             */
            std::vector<long long int> get_k_nearest_neighbors_indices(const CoordinateType *coordinates, unsigned int n) const {
                if (!m_tree_structure_built) {
                    throw std::runtime_error("KDTree::get_k_nearest_neighbors_indices: tree structure not built.");
                }
                if (n > m_nodes.size()) {
                    throw std::runtime_error("KDTree::get_k_nearest_neighbors_indices: n must be smaller than the number of nodes.");
                }

                std::vector<std::tuple<long long int, double>> indices_and_distances;
                get_n_closest_nodes_recursive(coordinates, &indices_and_distances, n, m_root_node_index);

                std::vector<long long int> result;
                for (const auto &index_and_distance : indices_and_distances)  {
                    result.push_back(std::get<0>(index_and_distance));
                }
                return result;
            };

            /**
             * @brief Get vector of indices of all points closer than "distance" to the query point.
             *
             */
            std::vector<long long int> get_nodes_closer_than_x(const CoordinateType *coordinates, const double distance) const {
                if (!m_tree_structure_built) {
                    throw std::runtime_error("KDTree::get_nodes_closer_than_x: tree structure not built.");
                }

                std::vector<std::tuple<long long int, double>> indices_and_distances;
                get_nodes_closer_than_x_recursive(coordinates, &indices_and_distances, distance*distance, m_root_node_index);

                std::vector<long long int> result;
                for (const auto &index_and_distance : indices_and_distances)  {
                    result.push_back(std::get<0>(index_and_distance));
                }
                return result;
            }


            /**
             * @brief Create the tree structure from the added nodes. After this, no more nodes can be added.
             *
             */
            void build_tree_structure()   {
                if (m_tree_structure_built) {
                    return;
                }
                std::vector<long long int> node_indices;
                for (unsigned long long int i = 0; i < m_nodes.size(); ++i)  {
                    node_indices.push_back(i);
                }

                m_root_node_index = build_node(node_indices, 0);
                m_tree_structure_built = true;
            };


            const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &get_point(long long int index) const {
                return m_nodes[index];
            };

        protected:
            bool m_tree_structure_built = false;
            std::vector<KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType>> m_nodes;
            long long int m_root_node_index = -1;
            unsigned int m_n_dim = 0;

            /**
             * @brief Takes a vector of node indices and builds a root node and its child nodes from them. Returns the index of the root node.
             *
             * @param node_indices - vector of node indices
             * @param split_axis - axis along which to split the nodes
             * @return long long int - index of the build node ("root" node)
             */
            long long int build_node(const std::vector<long long int> &node_indices, const int split_axis)    {
                if (node_indices.size() == 0)   {
                    return -1;
                }
                if (node_indices.size() == 1)   {
                    m_nodes[node_indices[0]].m_split_axis = split_axis;
                    return node_indices[0];
                }

                std::vector<std::tuple<long long int, CoordinateType>> node_indices_and_coordinates;
                for (long long int node_index : node_indices)  {
                    node_indices_and_coordinates.push_back(std::make_tuple(node_index, m_nodes[node_index].m_coordinates[split_axis]));
                }

                std::sort(node_indices_and_coordinates.begin(), node_indices_and_coordinates.end(), [](const std::tuple<long long int, CoordinateType> &a, const std::tuple<long long int, CoordinateType> &b) {
                    return std::get<1>(a) < std::get<1>(b);
                });

                const long long int median_node_index = std::get<0>(node_indices_and_coordinates[node_indices_and_coordinates.size() / 2]);
                const CoordinateType median_coordinate = std::get<1>(node_indices_and_coordinates[node_indices_and_coordinates.size() / 2]);

                node_indices_and_coordinates.clear();

                m_nodes[median_node_index].m_split_axis = split_axis;

                std::vector<long long int> left_node_indices;
                std::vector<long long int> right_node_indices;
                for (const long long int i_node : node_indices)  {
                    if (i_node == median_node_index)    {
                        continue;
                    }
                    if (m_nodes[i_node].m_coordinates[split_axis] < median_coordinate)    {
                        left_node_indices.push_back(i_node);
                    }
                    else    {
                        right_node_indices.push_back(i_node);
                    }
                }

                m_nodes[median_node_index].m_index_child_left  = build_node(left_node_indices,  (split_axis + 1) % m_n_dim);
                m_nodes[median_node_index].m_index_child_right = build_node(right_node_indices, (split_axis + 1) % m_n_dim);

                return median_node_index;
            };


            static inline void update_index_and_distance_vector(std::vector<std::tuple<long long int, double>> *node_indices_and_distances, long long int node_index, double distance, unsigned int requested_neighbors) {
                if (node_indices_and_distances->size() == requested_neighbors)  {
                    if (distance >= std::get<1>(node_indices_and_distances->back())) {
                        return;
                    }
                }

                size_t index_in_vector_to_place = 0;
                for (size_t i = 0; i < node_indices_and_distances->size(); ++i) {
                    if (distance >= std::get<1>((*node_indices_and_distances)[i])) {
                        index_in_vector_to_place = i;
                        break;
                    }
                }

                if (node_indices_and_distances->size() < requested_neighbors) {
                    node_indices_and_distances->resize(node_indices_and_distances->size() + 1);
                }

                for (size_t i = node_indices_and_distances->size()-1; i > index_in_vector_to_place; --i) {
                    (*node_indices_and_distances)[i] = (*node_indices_and_distances)[i-1];
                }
                (*node_indices_and_distances)[index_in_vector_to_place] = std::tuple<long long int, double>(node_index, distance);
            };

            void get_n_closest_nodes_recursive( const CoordinateType *coordinates,
                                                std::vector<std::tuple<long long int, double>> *node_indices_and_distances,
                                                unsigned int n_neighbors,
                                                long long int node_index) const {

                if (node_index == -1)   {
                    return;
                }

                const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &node = m_nodes[node_index];
                const double distance_to_node = get_distance_squared(coordinates, node.m_coordinates);

                update_index_and_distance_vector(node_indices_and_distances, node_index, distance_to_node, n_neighbors);

                const bool go_to_left = coordinates[node.m_split_axis] < node.m_coordinates[node.m_split_axis];
                if (go_to_left) {
                    get_n_closest_nodes_recursive(coordinates, node_indices_and_distances, n_neighbors, node.m_index_child_left);
                }
                else    {
                    get_n_closest_nodes_recursive(coordinates, node_indices_and_distances, n_neighbors, node.m_index_child_right);
                }

                const double distance_to_split_axis = pow2(coordinates[node.m_split_axis] - node.m_coordinates[node.m_split_axis]);
                if (distance_to_split_axis < std::get<1>(node_indices_and_distances->back()) || node_indices_and_distances->size() < n_neighbors) {
                    if (go_to_left) {
                        get_n_closest_nodes_recursive(coordinates, node_indices_and_distances, n_neighbors, node.m_index_child_right);
                    }
                    else    {
                        get_n_closest_nodes_recursive(coordinates, node_indices_and_distances, n_neighbors, node.m_index_child_left);
                    }
                }
            };


            void get_nodes_closer_than_x_recursive( const CoordinateType *coordinates,
                                                std::vector<std::tuple<long long int, double>> *node_indices_and_distances,
                                                double distance_squared,
                                                long long int node_index) const {

                if (node_index == -1)   {
                    return;
                }

                const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &node = m_nodes[node_index];
                const double distance_to_node = get_distance_squared(coordinates, node.m_coordinates);

                if (distance_to_node < distance_squared) {
                    node_indices_and_distances->push_back(std::tuple<long long int, double>(node_index, distance_to_node));
                }

                const bool go_to_left = coordinates[node.m_split_axis] < node.m_coordinates[node.m_split_axis];
                if (go_to_left) {
                    get_nodes_closer_than_x_recursive(coordinates, node_indices_and_distances, distance_squared, node.m_left);
                }
                else    {
                    get_nodes_closer_than_x_recursive(coordinates, node_indices_and_distances, distance_squared, node.m_right);
                }

                const double distance_to_split_axis = pow2(coordinates[node.m_split_axis] - node.m_coordinates[node.m_split_axis]);
                if (distance_to_split_axis < distance_squared) {
                    if (go_to_left) {
                        get_nodes_closer_than_x_recursive(coordinates, node_indices_and_distances, distance_squared, node.m_right);
                    }
                    else    {
                        get_nodes_closer_than_x_recursive(coordinates, node_indices_and_distances, distance_squared, node.m_left);
                    }
                }
            };

            double get_distance_squared(const std::array<CoordinateType, NumberOfCoordinates> &coordinates_1, const std::array<CoordinateType, NumberOfCoordinates> &coordinates_2)   const    {
                double distance = 0.0;
                for (unsigned int i = 0; i < m_n_dim; ++i)  {
                    distance += (coordinates_1[i] - coordinates_2[i]) * (coordinates_1[i] - coordinates_2[i]);
                }
                return distance;
            };

            double get_distance_squared(const CoordinateType *coordinates_1, const std::array<CoordinateType, NumberOfCoordinates> &coordinates_2)   const    {
                double distance = 0.0;
                for (unsigned int i = 0; i < m_n_dim; ++i)  {
                    distance += (coordinates_1[i] - coordinates_2[i]) * (coordinates_1[i] - coordinates_2[i]);
                }
                return distance;
            };
    };


}