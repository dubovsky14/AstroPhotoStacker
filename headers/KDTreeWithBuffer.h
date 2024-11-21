#pragma once

#include "../headers/Common.h"
#include "../headers/KDTree.h"

#include<tuple>
#include<vector>
#include<map>
#include<string>
#include<memory>
#include<algorithm>
#include<fstream>
#include <stdlib.h>

namespace AstroPhotoStacker   {
    /**
     * @brief Class implementing k-dimensional binary tree, which can be used to search for the nearest neighbors of a point. It has and internal buffer for avoiding memory allocations, but it is thread-unsafe.
     *
     */
    template<typename CoordinateType, unsigned int NumberOfCoordinates, typename ValueType>
    class KDTreeWithBuffer : public  KDTree<CoordinateType, NumberOfCoordinates, ValueType> {
        public:
            using KDTree<CoordinateType, NumberOfCoordinates, ValueType>::m_n_dim;
            using KDTree<CoordinateType, NumberOfCoordinates, ValueType>::m_nodes;
            using KDTree<CoordinateType, NumberOfCoordinates, ValueType>::m_root_node_index;
            using KDTree<CoordinateType, NumberOfCoordinates, ValueType>::m_tree_structure_built;

            KDTreeWithBuffer()  = default;

            KDTreeWithBuffer(const KDTree<CoordinateType, NumberOfCoordinates, ValueType> &kd_tree) {
                m_n_dim = kd_tree.m_n_dim;
                m_nodes = kd_tree.m_nodes;
                m_root_node_index = kd_tree.m_root_node_index;
                m_tree_structure_built = kd_tree.m_tree_structure_built;
            };

            const std::vector<std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>> &get_k_nearest_neighbors_buffer(const CoordinateType *query_point, unsigned int n_points)    const   {
                m_buffer_result_vector_coordinates_and_values.resize(0);
                get_k_nearest_neighbors_indices_buffer(query_point, n_points);
                for (long long int index : m_buffer_indices)  {
                    const KDTreeNode<CoordinateType, NumberOfCoordinates, ValueType> &point = m_nodes[index];
                    m_buffer_result_vector_coordinates_and_values.push_back (
                        std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>   (
                            point.m_coordinates,
                            point.m_value
                        )
                    );
                }
                return m_buffer_result_vector_coordinates_and_values;
            };

            /**
             * @brief Get vector of indices of n closest points to the query point.
             *
             * @param coordinates of the query point
             * @param n - number of closest points to return
             */
            const std::vector<long long int> &get_k_nearest_neighbors_indices_buffer(const CoordinateType *coordinates, unsigned int n) const {
                if (!m_tree_structure_built) {
                    throw std::runtime_error("KDTreeWithBuffer::get_k_nearest_neighbors_indices_buffer: tree structure not built.");
                }
                if (n > m_nodes.size()) {
                    throw std::runtime_error("KDTreeWithBuffer::get_k_nearest_neighbors_indices_buffer: n must be smaller than the number of nodes.");
                }

                m_buffer_indices_and_distances.resize(0);
                this->get_n_closest_nodes_recursive(coordinates, &m_buffer_indices_and_distances, n, m_root_node_index);

                m_buffer_indices.resize(0);
                for (const auto &index_and_distance : m_buffer_indices_and_distances)  {
                    m_buffer_indices.push_back(std::get<0>(index_and_distance));
                }
                return m_buffer_indices;
            };

            /**
             * @brief Get vector of indices of all points closer than "distance" to the query point.
             *
             */
            const std::vector<long long int> &get_nodes_closer_than_x_buffer(const CoordinateType *coordinates, const double distance) const {
                if (!m_tree_structure_built) {
                    throw std::runtime_error("KDTreeWithBuffer::get_nodes_closer_than_x_buffer: tree structure not built.");
                }

                m_buffer_indices_and_distances.resize(0);
                this->get_nodes_closer_than_x_recursive(coordinates, &m_buffer_indices_and_distances, distance*distance, m_root_node_index);

                m_buffer_indices.resize(0);
                for (const auto &index_and_distance : m_buffer_indices_and_distances)  {
                    m_buffer_indices.push_back(std::get<0>(index_and_distance));
                }
                return m_buffer_indices;
            }

        protected:
            mutable std::vector<std::tuple<std::array<CoordinateType, NumberOfCoordinates>, ValueType>> m_buffer_result_vector_coordinates_and_values;
            mutable std::vector<long long int> m_buffer_indices;
            mutable std::vector<std::tuple<long long int, double>> m_buffer_indices_and_distances;
    };



}