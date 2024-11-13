#pragma once

#include "../headers/StackerBase.h"

#include "../headers/StackSettings.h"

#include <string>
#include <memory>

namespace AstroPhotoStacker {
    std::unique_ptr<StackerBase> create_stacker(const std::string &stacker_type, int number_of_colors, int width, int height, bool interpolate_colors = true);

    void configure_stacker(StackerBase* stacker, const StackSettings &settings);

    std::unique_ptr<StackerBase> create_stacker(const StackSettings &settings, int number_of_colors, int width, int height);
}
