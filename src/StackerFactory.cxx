#include "../headers/StackerFactory.h"

#include "../headers/StackerBase.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerMedian.h"
#include "../headers/StackerKappaSigmaClipping.h"
#include "../headers/StackerKappaSigmaMedian.h"
#include "../headers/StackerCutOffAverage.h"
#include "../headers/StackerMaximum.h"
#include "../headers/StackerMinimum.h"
#include "../headers/StackerCenter.h"
#include "../headers/StackerQuantil.h"
#include "../headers/StackerRMS.h"



using namespace AstroPhotoStacker;
using namespace std;

std::unique_ptr<StackerBase> AstroPhotoStacker::create_stacker(const std::string &stacker_type, int number_of_colors, int width, int height, bool interpolate_colors) {
    if (stacker_type == "average") {
        return std::make_unique<StackerMeanValue>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "median") {
        return std::make_unique<StackerMedian>(number_of_colors, width, height, interpolate_colors);
    } else if (stacker_type == "kappa-sigma mean") {
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
    else if (stacker_type == "center") {
        return std::make_unique<StackerCenter>(number_of_colors, width, height, interpolate_colors);
    }
    else if (stacker_type == "quantil") {
        return std::make_unique<StackerQuantil>(number_of_colors, width, height, interpolate_colors);
    }
    else if (stacker_type == "rms") {
        return std::make_unique<StackerRMS>(number_of_colors, width, height, interpolate_colors);
    }
    else {
        throw std::runtime_error("Unknown stacker type: " + stacker_type);
    }
};

void AstroPhotoStacker::configure_stacker(StackerBase* stacker, const StackSettings &settings)   {
    stacker->set_number_of_cpu_threads(settings.get_n_cpus());
    stacker->set_memory_usage_limit(settings.get_max_memory());

    const std::map<std::string, double> algorithm_specific_settings = settings.get_algorithm_specific_settings();
    for (const auto &pair : algorithm_specific_settings) {
        stacker->set_additional_setting(pair.first, pair.second);
    }
};

std::unique_ptr<StackerBase> AstroPhotoStacker::create_stacker(const StackSettings &settings, int number_of_colors, int width, int height) {
    bool interpolate_colors = settings.use_color_interpolation();
    std::unique_ptr<StackerBase> stacker = create_stacker(settings.get_stacking_algorithm(), number_of_colors, width, height, interpolate_colors);
    configure_stacker(stacker.get(), settings);
    return stacker;
};

