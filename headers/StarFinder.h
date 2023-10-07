#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <memory>
#include <iostream>
#include <algorithm>
#include <climits>


namespace AstroPhotoStacker {
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
        for (auto cluster : clusters)   {
            float x_sum = 0;
            float y_sum = 0;
            for (auto pixel : cluster)  {
                x_sum += std::get<0>(pixel);
                y_sum += std::get<1>(pixel);
            }
            const float x_mean = x_sum / cluster.size();
            const float y_mean = y_sum / cluster.size();
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
}