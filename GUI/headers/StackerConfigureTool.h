#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"

#include "../../headers/StackerBase.h"
#include "../../headers/StackerFactory.h"
#include "../../headers/StackSettings.h"


#include <string>
#include <memory>

/**
 * @brief Factory function for creating a stacker object with the given settings
 *
 * @param stack_settings stack settings
 * @param filelist_handler filelist handler
 * @return std::unique_ptr<AstroPhotoStacker::StackerBase> pointer to the stacker object
 */
std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const AstroPhotoStacker::StackSettings& stack_settings, const FilelistHandlerGUIInterface& filelist_handler);

/**
 * @brief Configure the stacker object with the given settings
 *
 * @param stacker pointer to the stacker object
 * @param settings stack settings
 */
void configure_stacker(AstroPhotoStacker::StackerBase* stacker, const AstroPhotoStacker::StackSettings &settings);