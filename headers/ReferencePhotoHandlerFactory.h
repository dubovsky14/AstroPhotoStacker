#pragma once

#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/ConfigurableAlgorithmSettings.h"
#include "../headers/InputFrame.h"

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <vector>




namespace AstroPhotoStacker   {

    class ReferencePhotoHandlerBase;

    struct ReferencePhotoHandlerFactoryFunctions {
        std::function<std::unique_ptr<ReferencePhotoHandlerBase>(const InputFrame &, const ConfigurableAlgorithmSettingsMap &)> create_function;

        std::function<ConfigurableAlgorithmSettings()> get_configuration_function;
    };

    class ReferencePhotoHandlerFactory {
        public:

            static ConfigurableAlgorithmSettings get_configurable_algorithm_settings(const std::string &alignment_method);

            static std::unique_ptr<ReferencePhotoHandlerBase> get_reference_photo_handler(const InputFrame &reference_frame, const std::string &alignment_method, const ConfigurableAlgorithmSettingsMap &configuration_map);

            static std::vector<std::string> get_available_alignment_methods();

        private:
            ReferencePhotoHandlerFactory() = default;

            static std::map<std::string, ReferencePhotoHandlerFactoryFunctions> s_factory_functions;
    };

}