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

bool ReferencePhotoHandlerPlanetaryZeroRotation::calculate_alignment(const InputFrame &input_frame, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation, float *ranking) const{
    const bool result = ReferencePhotoHandlerPlanetary::calculate_alignment(input_frame, shift_x, shift_y, rot_center_x, rot_center_y, rotation, ranking);
    *rotation = 0;
    return result;
};
