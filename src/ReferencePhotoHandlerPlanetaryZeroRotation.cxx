#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"

#include "../headers/AlignmentResultTranslationOnly.h"
#include "../headers/AlignmentResultPlateSolving.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

using namespace AstroPhotoStacker;


ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const InputFrame &input_frame, float threshold_fraction)  :
    ReferencePhotoHandlerPlanetary(input_frame, threshold_fraction) {};

ReferencePhotoHandlerPlanetaryZeroRotation::ReferencePhotoHandlerPlanetaryZeroRotation(const PixelType *brightness, int width, int height, float threshold_fraction) :
    ReferencePhotoHandlerPlanetary(brightness, width, height, threshold_fraction) {};


std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerPlanetaryZeroRotation::calculate_alignment(const InputFrame &input_frame) const{
    std::unique_ptr<AlignmentResultBase> plate_solving_result_base = ReferencePhotoHandlerPlanetary::calculate_alignment(input_frame);
    AlignmentResultPlateSolving* plate_solving_result = dynamic_cast<AlignmentResultPlateSolving*>(plate_solving_result_base.get());
    float shift_x, shift_y, rotation_center_x, rotation_center_y, rotation;
    plate_solving_result->get_parameters(&shift_x, &shift_y, &rotation_center_x, &rotation_center_y, &rotation);
    const float score = plate_solving_result->get_ranking_score();

    std::unique_ptr<AlignmentResultTranslationOnly> translation_only_result = std::make_unique<AlignmentResultTranslationOnly>(shift_x, shift_y);
    translation_only_result->set_ranking_score(score);
    translation_only_result->set_is_valid( plate_solving_result->is_valid() );

    return translation_only_result;
};
