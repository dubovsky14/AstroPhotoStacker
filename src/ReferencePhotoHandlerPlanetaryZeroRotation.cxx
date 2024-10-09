#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

using namespace AstroPhotoStacker;


ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const std::string &raw_file_address, float threshold_fraction)  :
    ReferencePhotoHandlerPlanetary(raw_file_address, threshold_fraction) {};

ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const unsigned short *brightness, int width, int height, float threshold_fraction) :
    ReferencePhotoHandlerPlanetary(brightness, width, height, threshold_fraction) {};

bool ReferencePhotoHandlerPlanetaryZeroRotation::calculate_alignment(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation, float *ranking) const{
    const bool result = ReferencePhotoHandlerPlanetary::calculate_alignment(file_address, shift_x, shift_y, rot_center_x, rot_center_y, rotation, ranking);
    *rotation = 0;
    return result;
};
