#include "../headers/ReferencePhotoHandlerFactory.h"

#include "../headers/ReferencePhotoHandlerBase.h"

#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/ReferencePhotoHandlerComet.h"

using namespace AstroPhotoStacker;
using namespace std;


ConfigurableAlgorithmSettings ReferencePhotoHandlerFactory::get_configurable_algorithm_settings(const std::string &alignment_method) {
    if (alignment_method == "planetary") {
        ReferencePhotoHandlerPlanetary handler_dummy;
        return handler_dummy.get_configurable_algorithm_settings();
    }
    else if (alignment_method == "planetary without rotation") {
        ReferencePhotoHandlerPlanetaryZeroRotation handler_dummy;
        return handler_dummy.get_configurable_algorithm_settings();
    }
    else if (alignment_method == "surface") {
        ReferencePhotoHandlerSurface handler_dummy;
        return handler_dummy.get_configurable_algorithm_settings();
    }
    else if (alignment_method == "stars") {
        ReferencePhotoHandlerStars handler_dummy;
        return handler_dummy.get_configurable_algorithm_settings();
    }
    else if (alignment_method == "comet") {
        ReferencePhotoHandlerComet handler_dummy;
        return handler_dummy.get_configurable_algorithm_settings();
    }
    else {
        throw std::runtime_error("Unknown alignment method: " + alignment_method);
    }
};

unique_ptr<ReferencePhotoHandlerBase> ReferencePhotoHandlerFactory::get_reference_photo_handler(const InputFrame &reference_frame, const std::string &alignment_method, const ConfigurableAlgorithmSettingsMap &configuration_map)   {
    if (alignment_method == "stars") {
        return make_unique<ReferencePhotoHandlerStars>(reference_frame, configuration_map);
    }
    else if (alignment_method == "stars with variable zoom") {
        return make_unique<ReferencePhotoHandlerStars>(reference_frame, configuration_map);
    }
    else if (alignment_method == "planetary") {
        return make_unique<ReferencePhotoHandlerPlanetary>(reference_frame, configuration_map);
    }
    else if (alignment_method == "planetary without rotation")    {
        return make_unique<ReferencePhotoHandlerPlanetaryZeroRotation>(reference_frame, configuration_map);
    }
    else if (alignment_method == "surface") {
        return make_unique<ReferencePhotoHandlerSurface>(reference_frame, configuration_map);
    }
    else if (alignment_method == "comet") {
        return  make_unique<ReferencePhotoHandlerComet>(reference_frame, configuration_map);
    }
    else {
        throw runtime_error("Invalid alignment method: " + alignment_method);
    }
};
