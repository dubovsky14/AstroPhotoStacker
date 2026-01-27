#include "../headers/CommonImageOperations.h"

#include <vector>

using namespace AstroPhotoStacker;
using namespace std;


unsigned short AstroPhotoStacker::get_otsu_threshold(const unsigned short *brightness, int n_pixels)    {
    // Compute histogram
    const int n_bins = 65536;
    std::vector<unsigned int> histogram(n_bins, 0);
    for (int i = 0; i < n_pixels; i++) {
        histogram[brightness[i]]++;
    }

    double sum = 0;
    for (int t = 0; t < n_bins; t++) {
        sum += t * histogram[t];
    }

    double sum_background = 0;
    unsigned int weight_sum_background = 0;
    unsigned int weight_sum_foreground = 0;

    double maximal_variation = 0;
    unsigned short threshold = 0;

    for (int threshold = 0; threshold < n_bins; threshold++) {
        weight_sum_background += histogram[threshold];                  // Weight Background
        if (weight_sum_background == 0) continue;

        weight_sum_foreground = n_pixels - weight_sum_background;       // Weight Foreground
        if (weight_sum_foreground == 0) break;

        sum_background += static_cast<double>(threshold * histogram[threshold]);

        double mean_background = sum_background / weight_sum_background;          // Mean Background
        double mean_foreground = (sum - sum_background) / weight_sum_foreground;  // Mean Foreground

        // Calculate Between Class Variance
        double var_between =    static_cast<double>(weight_sum_background) *
                                static_cast<double>(weight_sum_foreground) *
                                (mean_background - mean_foreground) * (mean_background - mean_foreground);
        // Check if new maximum found
        if (var_between > maximal_variation) {
            maximal_variation = var_between;
            threshold = threshold;
        }
    }
    return threshold;
};