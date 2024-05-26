#pragma once

#include "../headers/StackSettings.h"
#include "../headers/FilelistHandler.h"

#include "../../headers/StackerBase.h"
#include "../../headers/StackerFactory.h"


#include <string>
#include <memory>

/**
 * @brief Factory function for creating a stacker object with the given settings
 *
 * @param stack_settings stack settings
 * @param filelist_handler filelist handler
 * @return std::unique_ptr<AstroPhotoStacker::StackerBase> pointer to the stacker object
 */
std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const StackSettings& stack_settings, const FilelistHandler& filelist_handler);

/**
 * @brief Get the stacker name in backend format
 *
 * @param stacking_algorithm_app_name name of the stacking algorithm from the GUI application
 * @return std::string type of the stacker used in the backend
 */
std::string get_stacker_type_for_factory(const std::string& stacking_algorithm_app_name);

/**
 * @brief Configure the stacker object with the given settings
 *
 * @param stacker pointer to the stacker object
 * @param settings stack settings
 */
void configure_stacker(AstroPhotoStacker::StackerBase* stacker, const StackSettings &settings);