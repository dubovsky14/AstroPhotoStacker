#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <memory>
#include <iostream>
#include <algorithm>
#include <climits>
#include<cmath>

namespace AstroPhotoStacker {

    // for each cluster calculate the difference between maximum and minimum radius
    std::vector<std::tuple<float,float>> get_cluster_smearing_vector(const std::vector< std::vector<std::tuple<int, int> > > &clusters);

    /**
     * @brief Get the center of mass of the cluster
     *
     * @param cluster - vector of pixels in the cluster as tuples of x and y coordinates
     * @return std::tuple<float,float> - center of mass of the cluster
    */
    std::tuple<float,float> get_center_of_cluster(const std::vector<std::tuple<int,int>> &cluster);

    /**
     * @brief Fill the current cluster with pixels. If the pixels is below the threshold, stop. If the pixel is already visited, stop. If the pixel is above the threshold, add it to the cluster and recursively call the function for all neighboring pixels
     *
     * @tparam pixel_type - type of pixel values
     * @param brightness - array of pixel brightness values
     * @param width - width of the image
     * @param height - height of the image
     * @param x_pos - x position of the pixel
     * @param y_pos - y position of the pixel
     * @param current_cluster - vector of pixels in the current cluster
     * @param threshold - pixels with brightness below this value will not be added to the cluster
     * @param visited_pixels - map of visited pixels
    */
    template<typename pixel_type>
    void fill_cluster(  const pixel_type *brightness, int width, int height,
                        int x_pos, int y_pos,
                        std::vector<std::tuple<int, int> > *current_cluster,
                        float threshold, std::map<std::tuple<int, int>, char> *visited_pixels) {
        if (brightness[y_pos*width + x_pos] < threshold)   return;

        std::tuple<int, int> this_pixel(x_pos,y_pos);
        if (visited_pixels->find(this_pixel) != visited_pixels->end())    {
            return;
        }
        (*visited_pixels)[this_pixel] = 0;
        current_cluster->push_back(this_pixel);
        for (int y_shift = -1; y_shift <= 1; y_shift++) {
            int y_pos_new = y_pos + y_shift;
            if (y_pos_new < 0 || y_pos_new >= height) continue;
            for (int x_shift = -1; x_shift <= 1; x_shift++) {
                int x_pos_new = x_pos + x_shift;
                if (x_pos_new < 0 || x_pos_new >= width) continue;
                if (x_shift == 0 && y_shift ==0)    continue;
                fill_cluster(brightness, width, height, x_pos_new, y_pos_new, current_cluster, threshold, visited_pixels);
            }
        }
    };

    /**
     * @brief Get the clusters of pixels in the image
     *
     * @tparam pixel_type - type of pixel values
     * @param brightness - array of pixel brightness values
     * @param width - width of the image
     * @param height - height of the image
     * @param threshold - pixels with brightness below this value will not be added to the cluster
     * @return std::vector<std::vector<std::tuple<int, int> > > - vector of clusters, each cluster is a vector of tuples representing x and y coordinates of the pixels
    */
    template<typename pixel_type>
    std::vector< std::vector<std::tuple<int, int> > > get_clusters(const pixel_type *brightness, int width, int height, float threshold)  {
        std::vector< std::vector<std::tuple<int, int> > >  result;
        std::map<std::tuple<int, int>, char> visited_pixels;

        std::vector<std::tuple<int, int> >  this_cluster;
        for (int y_pos = 0; y_pos < height; y_pos++)    {
            for (int x_pos = 0; x_pos < width; x_pos++)    {
                fill_cluster(brightness, width, height, x_pos, y_pos, &this_cluster, threshold, &visited_pixels);
                if (this_cluster.size())    {
                    result.push_back(this_cluster);
                    this_cluster.clear();
                }
            }
        }
        return result;
    };

    /**
     * @brief Get the clusters of pixels in the image
     *
     * @tparam pixel_type - type of pixel values
     * @param brightness - array of pixel brightness values
     * @param width - width of the image
     * @param height - height of the image
     * @param threshold - pixels with brightness below this value will not be added to the cluster
     * @return std::vector<std::vector<std::tuple<int, int> > > - vector of clusters, each cluster is a vector of tuples representing x and y coordinates of the pixels
    */
    template<typename pixel_type>
    std::vector< std::vector<std::tuple<int, int> > > get_clusters_non_recursive(const pixel_type *brightness, int width, int height, float threshold)  {
        // yes, this function is a mess and it can be done elegantly with recursion, but for large objects (i.e. Moon) it will cause stack overflow

        std::vector< std::vector<std::tuple<int, int> > >  result;
        std::vector< std::vector<std::tuple<int, int> > >  active_clusters;
        std::vector< std::vector<std::tuple<int, int> > >  active_clusters_previous_line;
        std::vector< std::vector<std::tuple<int, int> > >  active_clusters_this_line;
        std::vector<bool> cluster_is_active;

        auto get_active_cluster_index = [&active_clusters_previous_line, &active_clusters_this_line] (int x, int y) -> std::tuple<int,int> {
            std::tuple<int,int> result(-1,-1);

            for (int i_cluster = 0; i_cluster < int(active_clusters_this_line.size()); i_cluster++) {
                for (const std::tuple<int, int> &pixel : active_clusters_this_line[i_cluster]) {
                    if (std::get<0>(pixel) == (x-1) && std::get<1>(pixel) == y) {     // left
                        std::get<0>(result) = i_cluster;
                        break;
                    }
                }
            }
            for (int i_cluster = 0; i_cluster < int(active_clusters_previous_line.size()); i_cluster++) {
                if (std::get<0>(result) == i_cluster)  continue;
                for (const std::tuple<int, int> &pixel : active_clusters_previous_line[i_cluster]) {
                    if ((std::get<0>(pixel) == x-1 && std::get<1>(pixel) == y-1) || // top left
                        (std::get<0>(pixel) == x   && std::get<1>(pixel) == y-1) || // top
                        (std::get<0>(pixel) == x+1 && std::get<1>(pixel) == y-1)) { // top right

                        if (std::get<0>(result) < 0) {
                            std::get<0>(result) = i_cluster;
                            break;
                        }
                        else {
                            std::get<1>(result) = i_cluster;
                            return result;
                        }
                    }
                }
            }
            return result;
        };

        auto merge_vectors = []( std::vector<std::vector<std::tuple<int, int> > > *source_vector, unsigned int source_index,
                            std::vector<std::vector<std::tuple<int, int> > > *destination_vector, unsigned int destination_index) -> void {

            if (source_index == destination_index) {
                const std::string error_message = "Error: source and destination indices are the same: " + std::to_string(source_index) + "\n";
                std::cout << error_message;
            }
            for (const std::tuple<int, int> &pixel : source_vector->at(source_index)) {
                destination_vector->at(destination_index).push_back(pixel);
            }
            source_vector->erase(source_vector->begin() + source_index);
        };

        for (int y_pos = 0; y_pos < height; y_pos++)    {
            active_clusters_previous_line = active_clusters_this_line;
            active_clusters_this_line.clear();
            active_clusters_this_line.resize(active_clusters.size());
            for (unsigned int i_cluster = 0; i_cluster < cluster_is_active.size(); i_cluster++) {
                cluster_is_active[i_cluster] = false;
            }

            for (int x_pos = 0; x_pos < width; x_pos++)    {
                if (brightness[y_pos*width + x_pos] < threshold)   continue;

                const std::tuple<int,int> active_cluster_indices = get_active_cluster_index(x_pos, y_pos);
                const int index_0 = std::get<0>(active_cluster_indices);
                const int index_1 = std::get<1>(active_cluster_indices);
                if (index_0 == -1) {
                    std::vector<std::tuple<int, int> > new_cluster;
                    new_cluster.push_back(std::make_tuple(x_pos, y_pos));
                    active_clusters.push_back(new_cluster);
                    active_clusters_this_line.push_back(new_cluster);
                    active_clusters_previous_line.push_back(std::vector<std::tuple<int, int> >());
                    cluster_is_active.push_back(true);
                } else {
                    active_clusters[index_0].push_back(std::make_tuple(x_pos, y_pos));
                    active_clusters_this_line[index_0].push_back(std::make_tuple(x_pos, y_pos));
                    cluster_is_active[index_0] = true;
                }

                // merge clusters
                if (index_1 != -1)   {
                    cluster_is_active.erase(cluster_is_active.begin() + index_1);
                    merge_vectors(&active_clusters, index_1, &active_clusters, index_0);
                    merge_vectors(&active_clusters_this_line, index_1, &active_clusters_this_line, index_0);
                    merge_vectors(&active_clusters_previous_line, index_1, &active_clusters_previous_line, index_0);
                }
            }

            // drop inactive clusters
            for (int i_cluster = 0; i_cluster < int(active_clusters.size()); i_cluster++) {
                if (!cluster_is_active[i_cluster]) {
                    result.push_back(active_clusters[i_cluster]);
                    active_clusters.erase(active_clusters.begin() + i_cluster);
                    active_clusters_this_line.erase(active_clusters_this_line.begin() + i_cluster);
                    cluster_is_active.erase(cluster_is_active.begin() + i_cluster);
                    i_cluster--;
                }
            }
        }

        // take care of the last line
        for (std::vector<std::tuple<int,int> > &cluster : active_clusters) {
            result.push_back(cluster);
        }

        std::sort(result.begin(), result.end(), [](const std::vector<std::tuple<int, int> > &a, const std::vector<std::tuple<int, int> > &b) {
            return a.size() > b.size();
        });

        return result;
    };

    /**
     * @brief Get the stars in the image as a vector of tuples of x and y coordinates and number of pixels in the star
     *
     * @tparam pixel_type - type of pixel values
     * @param brightness - array of pixel brightness values
     * @param width - width of the image
     * @param height - height of the image
     * @param threshold - pixels with brightness below this value will not be added to the cluster
     * @return std::vector<std::tuple<float, float,int>> - vector of stars, each star is a tuple of x and y coordinates and number of pixels in the star
    */
    template<typename pixel_type>
    std::vector<std::tuple<float, float,int>> get_stars(const pixel_type *brightness, int width, int height, pixel_type threshold) {
        std::vector<std::tuple<float, float, int>> stars;
        std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters_non_recursive(brightness, width, height, threshold);

        for (auto cluster : clusters)   {
            auto [x_mean, y_mean] = get_center_of_cluster(cluster);
            const int n_pixels = cluster.size();
            stars.push_back(std::make_tuple(x_mean, y_mean, n_pixels));
        }
        return stars;
    };

    /**
     * @brief Get the threshold value for the given fraction of the brightest pixels
     *
     * @tparam pixel_type - type of pixel values
     * @param brightness - array of pixel brightness values
     * @param array_size - size of the array
     * @param fraction - fraction of the brightest pixels
     * @return pixel_type - threshold value above which there is "fraction" of all pixels
    */
    template<typename pixel_type>
    pixel_type get_threshold_value(const pixel_type *brightness, unsigned int array_size, float fraction)    {
        std::vector<unsigned int> histogram(USHRT_MAX, 0);

        for (unsigned int i = 0; i < array_size; i++)    {
            histogram[brightness[i]]++;
        }

        unsigned int sum = 0;
        unsigned int threshold_index = 0;
        for (unsigned int i = 0; i < USHRT_MAX; i++)    {
            sum += histogram[i];
            if (sum > (1-fraction) * array_size)   {
                return i;
            }
        }
        return threshold_index;
    };

    /**
     * @brief Sort the stars by size (number of pixels belonging to that star)
     *
     * @param stars - vector of stars, each star is a tuple of x and y coordinates and number of pixels in the star
    */
    void sort_stars_by_size(std::vector<std::tuple<float, float,int> > *stars);

    /**
     * @brief Keep only stars above the given size
     *
     * @param stars - vector of stars, each star is a tuple of x and y coordinates and number of pixels in the star
     * @param min_size - minimum size (number of pixels) of the star
    */
    void keep_only_stars_above_size(std::vector<std::tuple<float, float,int> > *stars, int min_size);

    /**
     * @brief Does the pixel belong to the border of the cluster?
     *
     * @param pixel - tuple of x and y coordinates of the pixel
     * @param cluster - vector of pixels in the cluster as tuples of x and y coordinates
     * @return true - if the pixel belongs to the border of the cluster
    */
    bool is_border_pixel(const std::tuple<int,int> &pixel, const std::vector<std::tuple<int,int> > &cluster);

    /**
     * @brief Get size of a 2D vector (represented by tuple<float,float>) squared
     *
     * @param vec - tuple of x and y coordinates of the point
     * @return float - size of the vector squared
    */
    float get_distance_squared(const std::tuple<float,float> &vec);
}

namespace MyTupleArithmetics {
    template<typename T1, typename T2>
    auto operator+(const std::tuple<T1,T1> &a, const std::tuple<T2,T2> &b)  {
        return std::make_tuple(std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b));
    };

    template<typename T1, typename T2>
    auto operator-(const std::tuple<T1,T1> &a, const std::tuple<T2,T2> &b)  {
        return std::make_tuple(std::get<0>(a) - std::get<0>(b), std::get<1>(a) - std::get<1>(b));
    };

    template<typename T, typename NumberType>
    std::tuple<T,T> operator*(const std::tuple<T,T> &a, NumberType b)  {
        return std::make_tuple(std::get<0>(a) * b, std::get<1>(a) * b);
    };

    template<typename T, typename NumberType>
    std::tuple<T,T> operator*(NumberType b, const std::tuple<T,T> &a)  {
        return std::make_tuple(std::get<0>(a) * b, std::get<1>(a) * b);
    };
}