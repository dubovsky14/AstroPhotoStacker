#include "../headers/StackerFactory.h"

#include "../headers/StackerBase.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerMedian.h"
#include "../headers/StackerKappaSigmaClipping.h"
#include "../headers/StackerKappaSigmaMedian.h"
#include "../headers/StackerCutOffAverage.h"
#include "../headers/StackerMaximum.h"
#include "../headers/StackerMinimum.h"
#include "../headers/StackerWeightedBestScore.h"




using namespace AstroPhotoStacker;
using namespace std;

std::unique_ptr<StackerBase> AstroPhotoStacker::create_stacker(const std::string &stacker_type, int number_of_colors, int width, int height, bool interpolate_colors) {
    if (stacker_type == "average") {
        return std::make_unique<StackerMeanValue>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "median") {
        return std::make_unique<StackerMedian>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "kappa-sigma clipping") {
        return std::make_unique<StackerKappaSigmaClipping>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "kappa-sigma median") {
        return std::make_unique<StackerKappaSigmaMedian>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "cut-off average") {
        return std::make_unique<StackerCutOffAverage>(number_of_colors, width, height, interpolate_colors);
    }
    else if (stacker_type == "maximum") {
        return std::make_unique<StackerMaximum>(number_of_colors, width, height, interpolate_colors);
    }
    else if (stacker_type == "minimum") {
        return std::make_unique<StackerMinimum>(number_of_colors, width, height, interpolate_colors);
    }
    else if (stacker_type == "best score") {
        return std::make_unique<StackerWeightedBestScore>(number_of_colors, width, height, interpolate_colors);
    }
    else {
        throw std::runtime_error("Unknown stacker type: " + stacker_type);
    }
};

void AstroPhotoStacker::configure_stacker(StackerBase* stacker, const StackSettings &settings)   {
    // kappa-sigma
    StackerKappaSigmaBase *stacker_kappa_sigma = dynamic_cast<StackerKappaSigmaBase*>(stacker);
    if (stacker_kappa_sigma != nullptr) {
        stacker_kappa_sigma->set_kappa(settings.get_kappa());
        stacker_kappa_sigma->set_number_of_iterations(settings.get_kappa_sigma_iter());
    }

    stacker->set_number_of_cpu_threads(settings.get_n_cpus());

    // Memory limit
    if (dynamic_cast<StackerMeanValue*>(stacker) == nullptr) {
        stacker->set_memory_usage_limit(settings.get_max_memory());
    }

    // cut-off average
    if (dynamic_cast<StackerCutOffAverage*>(stacker) != nullptr) {
        StackerCutOffAverage *stacker_cut_off_average = dynamic_cast<StackerCutOffAverage*>(stacker);
        stacker_cut_off_average->set_tail_fraction_to_cut_off(settings.get_cut_off_tail_fraction());
    }
};

std::unique_ptr<StackerBase> AstroPhotoStacker::create_stacker(const StackSettings &settings, int number_of_colors, int width, int height) {
    bool interpolate_colors = settings.use_color_interpolation();
    std::unique_ptr<StackerBase> stacker = create_stacker(settings.get_stacking_algorithm(), number_of_colors, width, height, interpolate_colors);
    configure_stacker(stacker.get(), settings);
    return stacker;
};

