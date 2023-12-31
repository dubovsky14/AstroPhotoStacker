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

    std::tuple<float,float> get_center_of_cluster(const std::vector<std::tuple<int,int>> &cluster);

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

    template<typename pixel_type>
    std::vector<std::tuple<float, float,int>> get_stars(const pixel_type *brightness, int width, int height, pixel_type threshold) {
        std::vector<std::tuple<float, float, int>> stars;
        std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters(brightness, width, height, threshold);

        auto cluster_shifts = get_cluster_smearing_vector(clusters);
        for (auto cluster : clusters)   {
            auto [x_mean, y_mean] = get_center_of_cluster(cluster);
            const int n_pixels = cluster.size();
            stars.push_back(std::make_tuple(x_mean, y_mean, n_pixels));
        }
        return stars;
    };

    template<typename pixel_type>
    pixel_type get_threshold_value(const pixel_type *brightness, unsigned int array_size, float fraction)    {
        std::unique_ptr<unsigned int[]> histogram = std::unique_ptr<unsigned int[]>(new unsigned int[USHRT_MAX]);
        for (unsigned int i = 0; i < USHRT_MAX; i++)    {
            histogram[i] = 0;
        }

        for (unsigned int i = 0; i < array_size; i++)    {
            histogram[brightness[i]]++;
        }

        unsigned int sum = 0;
        unsigned int threshold_index = 0;
        for (unsigned int i = 0; i < USHRT_MAX; i++)    {
            sum += histogram[i];
            if (sum > (1-fraction) * array_size)   {
                threshold_index = i;
                break;
            }
        }
        return threshold_index;
    };

    void sort_stars_by_size(std::vector<std::tuple<float, float,int> > *stars);

    void keep_only_stars_above_size(std::vector<std::tuple<float, float,int> > *stars, int min_size);

    bool is_border_pixel(const std::tuple<int,int> &pixel, const std::vector<std::tuple<int,int> > &cluster);

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