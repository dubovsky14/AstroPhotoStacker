#pragma once

#include "../headers/StackerBase.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerMedian.h"
#include "../headers/StackerKappaSigmaClipping.h"
#include "../headers/StackerKappaSigmaMedian.h"
#include "../headers/StackerCutOffAverage.h"
#include "../headers/StackerMaximum.h"
#include "../headers/StackerMinimum.h"
#include "../headers/StackerWeightedBestScore.h"

#include <string>
#include <memory>

namespace AstroPhotoStacker {
    inline std::unique_ptr<StackerBase> create_stacker(const std::string &stacker_type, int number_of_colors, int width, int height, bool interpolate_colors = true) {
        if (stacker_type == "mean") {
            return std::make_unique<StackerMeanValue>(number_of_colors, width, height, interpolate_colors);
        } else if (stacker_type == "median") {
            return std::make_unique<StackerMedian>(number_of_colors, width, height, interpolate_colors);
        } else if (stacker_type == "kappa_sigma_clipping") {
            return std::make_unique<StackerKappaSigmaClipping>(number_of_colors, width, height, interpolate_colors);
        } else if (stacker_type == "kappa_sigma_median") {
            return std::make_unique<StackerKappaSigmaMedian>(number_of_colors, width, height, interpolate_colors);
        } else if (stacker_type == "cut_off_average") {
            return std::make_unique<StackerCutOffAverage>(number_of_colors, width, height, interpolate_colors);
        }
        else if (stacker_type == "maximum") {
            return std::make_unique<StackerMaximum>(number_of_colors, width, height, interpolate_colors);
        }
        else if (stacker_type == "minimum") {
            return std::make_unique<StackerMinimum>(number_of_colors, width, height, interpolate_colors);
        }
        else if (stacker_type == "best_score") {
            return std::make_unique<StackerWeightedBestScore>(number_of_colors, width, height, interpolate_colors);
        }
        else {
            throw std::runtime_error("Unknown stacker type: " + stacker_type);
        }
    }
}