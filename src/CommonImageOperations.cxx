#include "../headers/CommonImageOperations.h"

#include <vector>
#include <limits>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


unsigned short AstroPhotoStacker::get_otsu_threshold(const unsigned short *brightness, int n_pixels)    {
    const int n_bins = 65536;
    std::vector<unsigned int> histogram(n_bins, 0);
    for (int i = 0; i < n_pixels; i++) {
        histogram[brightness[i]]++;
    }

    double sum(0), sum2(0);
    for (int t = 0; t < n_bins; t++) {
        sum  += t * histogram[t];
        sum2 += t * t * histogram[t];
    }

    double sum_background = 0;
    double sum2_background = 0;
    unsigned int weight_sum_background = 0;
    unsigned int weight_sum_foreground = 0;

    double minimal_variation = std::numeric_limits<double>::max();
    unsigned short optimal_threshold = 0;

    for (int threshold = 0; threshold < n_bins; threshold++) {
        weight_sum_background += histogram[threshold];
        if (weight_sum_background == 0) continue;

        weight_sum_foreground = n_pixels - weight_sum_background;
        if (weight_sum_foreground == 0) break;

        sum_background += static_cast<double>(threshold * histogram[threshold]);
        sum2_background += static_cast<double>(threshold * threshold * histogram[threshold]);

        const double mean_background = sum_background / weight_sum_background;
        const double mean2_background = sum2_background / weight_sum_background;

        const double mean_foreground = (sum - sum_background) / weight_sum_foreground;
        const double mean2_foreground = (sum2 - sum2_background) / weight_sum_foreground;

        const double variance_background = mean2_background - mean_background * mean_background;
        const double variance_foreground = mean2_foreground - mean_foreground * mean_foreground;

        const double inter_class_variance = (weight_sum_background * variance_background + weight_sum_foreground * variance_foreground);
        if (inter_class_variance < minimal_variation) {
            minimal_variation = inter_class_variance;
            optimal_threshold = threshold;
        }
    }
    return optimal_threshold;
};