#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

using namespace AstroPhotoStacker;


ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const InputFrame &input_frame, float threshold_fraction)  :
    ReferencePhotoHandlerPlanetary(input_frame, threshold_fraction) {};

ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const unsigned short *brightness, int width, int height, float threshold_fraction) :
    ReferencePhotoHandlerPlanetary(brightness, width, height, threshold_fraction) {};


PlateSolvingResult ReferencePhotoHandlerPlanetaryZeroRotation::calculate_alignment(const InputFrame &input_frame, float *ranking) const{
    PlateSolvingResult result = ReferencePhotoHandlerPlanetary::calculate_alignment(input_frame, ranking);
    result.rotation = 0;
    return result;
};
