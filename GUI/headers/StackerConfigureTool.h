#pragma once

#include "../headers/StackSettings.h"
#include "../headers/FilelistHandler.h"

#include "../../headers/StackerBase.h"
#include "../../headers/StackerFactory.h"


#include <string>
#include <memory>

std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const StackSettings& stack_settings, const FilelistHandler& filelist_handler);

std::string get_stacker_type_for_factory(const std::string& stacking_algorithm_app_name);

void configure_stacker(AstroPhotoStacker::StackerBase* stacker, const StackSettings &settings);