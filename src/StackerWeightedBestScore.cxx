#include "../headers/StackerWeightedBestScore.h"
#include "../headers/CalibratedPhotoHandler.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

StackerWeightedBestScore::StackerWeightedBestScore(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerWeightedMedian(number_of_colors, width, height, interpolate_colors)   {
};

double StackerWeightedBestScore::get_stacked_value_from_pixel_array(tuple<short,ScoreType> *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }

    sort(ordered_array_begin, ordered_array_begin + number_of_stacked_pixels, [](const tuple<short,ScoreType> &a, const tuple<short,ScoreType> &b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    return std::get<0>(ordered_array_begin[number_of_stacked_pixels-1]);
};
