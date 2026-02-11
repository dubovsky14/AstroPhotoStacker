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
        std::vector<int> cluster_indices(width*height, -1);
        unsigned int n_clusters = 0;
        std::map<int, int> cluster_index_mapping;

        // The problem is that given pixel might not any top or left neighbor from known cluster, but as you go through the line, you might find a pixel with a neighboring cluster - we will take care of merging later
        auto add_cluster_mapping = [&cluster_index_mapping] (int from, int to) -> void {
            if (from < to) std::swap(from, to);
            if (cluster_index_mapping.find(from) != cluster_index_mapping.end())    {
                const int previosly_mapped_to = cluster_index_mapping[from];
                if (previosly_mapped_to > to) {
                    cluster_index_mapping[from] = to;
                }
            }
            else {
                cluster_index_mapping[from] = to;
            }
        };

        auto get_cluster_index = [&cluster_indices, &n_clusters, width, &add_cluster_mapping] (int x, int y) -> int {
            int result = -1;
            // check pixel on the left
            if (x > 0) {
                if (cluster_indices[y*width + x-1] != -1) {
                    result = cluster_indices[y*width + x-1];
                }
            }

            // check pixels on the top
            if (y > 0) {
                if (x > 0) {
                    const int cluster_index = cluster_indices[(y-1)*width + x-1];
                    if (cluster_index != -1) {
                        if (result != -1){
                            add_cluster_mapping(result, cluster_index);
                        }
                        result = result < 0 ? cluster_index : std::min(result, cluster_index);
                    }
                }
                if (cluster_indices[(y-1)*width + x] != -1) {
                    const int cluster_index = cluster_indices[(y-1)*width + x];
                    if (result != -1) {
                        add_cluster_mapping(result, cluster_index);
                    }
                        result = result < 0 ? cluster_index : std::min(result, cluster_index);
                }
                if (x < (width - 1)) {
                    const int cluster_index = cluster_indices[(y-1)*width + x+1];
                    if (cluster_index != -1) {
                        if (result != -1) {
                            add_cluster_mapping(result, cluster_index);
                        }
                        result = result < 0 ? cluster_index : std::min(result, cluster_index);
                    }
                }
            }

            if (result != -1)   return result;

            // no neighboring pixels belong to a cluster, create a new cluster
            return n_clusters++;
        };

        for (int y_pos = 0; y_pos < height; y_pos++)    {
            for (int x_pos = 0; x_pos < width; x_pos++)    {
                if (brightness[y_pos*width + x_pos] < threshold)   continue;

                const int index = get_cluster_index(x_pos, y_pos);
                cluster_indices[y_pos*width + x_pos] = index;
            }
        }


        // calculate final mapping of cluster indices
        for (unsigned int i = 0; i < n_clusters; i++)    {
            if (cluster_index_mapping.find(i) != cluster_index_mapping.end())    {
                int mapped_to = cluster_index_mapping[i];
                while (cluster_index_mapping.find(mapped_to) != cluster_index_mapping.end())    {
                    if (mapped_to == cluster_index_mapping[mapped_to] || mapped_to < cluster_index_mapping[mapped_to]) {
                        break;
                    }
                    mapped_to = cluster_index_mapping[mapped_to];
                }
                cluster_index_mapping[i] = mapped_to;
            }
            else {
                cluster_index_mapping[i] = i;
            }
        }

        std::vector< std::vector<std::tuple<int, int> > >  result(n_clusters);
        for (int y_pos = 0; y_pos < height; y_pos++)    {
            for (int x_pos = 0; x_pos < width; x_pos++)    {
                if (cluster_indices[y_pos*width + x_pos] != -1) {
                    const int mapped_to = cluster_index_mapping[cluster_indices[y_pos*width + x_pos]];
                    result[mapped_to].push_back(std::make_tuple(x_pos, y_pos));
                }
            }
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
        std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters(brightness, width, height, threshold);

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