#include "../headers/ReferencePhotoHandlerFactory.h"

#include "../headers/ReferencePhotoHandlerBase.h"

#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/ReferencePhotoHandlerComet.h"

using namespace AstroPhotoStacker;
using namespace std;

std::map<std::string, ReferencePhotoHandlerFactoryFunctions> ReferencePhotoHandlerFactory::s_factory_functions = {
    {"stars",
        {
            [](const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) {
                return make_unique<ReferencePhotoHandlerStars>(input_frame, configuration_map);
            },
            []() {
                ReferencePhotoHandlerStars handler_dummy;
                return handler_dummy.get_configurable_algorithm_settings();
            }
        }
    },
    {"planetary",
        {
            [](const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) {
                return make_unique<ReferencePhotoHandlerPlanetary>(input_frame, configuration_map);
            },
            []() {
                ReferencePhotoHandlerPlanetary handler_dummy;
                return handler_dummy.get_configurable_algorithm_settings();
            }
        }
    },
    {"planetary without rotation",
        {
            [](const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) {
                return make_unique<ReferencePhotoHandlerPlanetaryZeroRotation>(input_frame, configuration_map);
            },
            []() {
                ReferencePhotoHandlerPlanetaryZeroRotation handler_dummy;
                return handler_dummy.get_configurable_algorithm_settings();
            }
        }
    },
    {"surface",
        {
            [](const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) {
                return make_unique<ReferencePhotoHandlerSurface>(input_frame, configuration_map);
            },
            []() {
                ReferencePhotoHandlerSurface handler_dummy;
                return handler_dummy.get_configurable_algorithm_settings();
            }
        }
    },
    {"comet",
        {
            [](const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) {
                return make_unique<ReferencePhotoHandlerComet>(input_frame, configuration_map);
            },
            []() {
                ReferencePhotoHandlerComet handler_dummy;
                return handler_dummy.get_configurable_algorithm_settings();
            }
        }
    }
};


ConfigurableAlgorithmSettings ReferencePhotoHandlerFactory::get_configurable_algorithm_settings(const std::string &alignment_method) {
    if (s_factory_functions.find(alignment_method) != s_factory_functions.end()) {
        return s_factory_functions.at(alignment_method).get_configuration_function();
    }
    else {
        throw runtime_error("Invalid alignment method: " + alignment_method);
    }
};

unique_ptr<ReferencePhotoHandlerBase> ReferencePhotoHandlerFactory::get_reference_photo_handler(const InputFrame &reference_frame, const std::string &alignment_method, const ConfigurableAlgorithmSettingsMap &configuration_map)   {
    if (s_factory_functions.find(alignment_method) != s_factory_functions.end()) {
        return s_factory_functions.at(alignment_method).create_function(reference_frame, configuration_map);
    }
    else {
        throw runtime_error("Invalid alignment method: " + alignment_method);
    }
};

std::vector<std::string> ReferencePhotoHandlerFactory::get_available_alignment_methods() {
    std::vector<std::string> methods;
    for (const auto &pair : s_factory_functions) {
        methods.push_back(pair.first);
    }
    return methods;
};