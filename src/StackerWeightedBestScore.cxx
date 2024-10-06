#include "../headers/StackerWeightedBestScore.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerWeightedBestScore::StackerWeightedBestScore(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerWeightedMedian(number_of_colors, width, height, interpolate_colors)   {
};

double StackerWeightedBestScore::get_stacked_value_from_pixel_array(tuple<short,ScoreType> *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }

    unsigned int max_score_index = 0;
    ScoreType max_score = std::get<1>(ordered_array_begin[0]);

    for (unsigned int i_pixel = 1; i_pixel < number_of_stacked_pixels; i_pixel++) {
        if (std::get<1>(ordered_array_begin[i_pixel]) > max_score) {
            max_score = std::get<1>(ordered_array_begin[i_pixel]);
            max_score_index = i_pixel;
        }
    }
    return std::get<0>(ordered_array_begin[max_score_index]);
};
