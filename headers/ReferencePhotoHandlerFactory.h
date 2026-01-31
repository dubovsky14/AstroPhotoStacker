#pragma once

#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/ConfigurableAlgorithmSettings.h"
#include "../headers/InputFrame.h"

#include <memory>
#include <string>



namespace AstroPhotoStacker   {

    class ReferencePhotoHandlerBase;

    class ReferencePhotoHandlerFactory {
        public:

            static ConfigurableAlgorithmSettings get_configurable_algorithm_settings(const std::string &alignment_method);

            static std::unique_ptr<ReferencePhotoHandlerBase> get_reference_photo_handler(const InputFrame &reference_frame, const std::string &alignment_method, const ConfigurableAlgorithmSettingsMap &configuration_map);
    };

}