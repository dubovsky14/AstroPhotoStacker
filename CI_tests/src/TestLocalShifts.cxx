
#include "../headers/TestLocalShifts.h"

#include "../../headers/ReferencePhotoHandlerSurface.h"
#include "../../headers/LocalShiftsHandler.h"
#include "../../headers/ConfigurableAlgorithmSettings.h"

#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

TestResult AstroPhotoStacker::test_predefined_alignment_boxes(  const InputFrame &reference_frame,
                                                                const InputFrame &alternative_frame,
                                                                const std::vector<std::tuple<int,int,int,int>> &expected_shifts)    {

    string error_message;

    float max_shift_magnitude = 0.0f;
    for (const std::tuple<int,int,int,int> &expected_shift : expected_shifts) {
        const int expected_dx = std::get<2>(expected_shift);
        const int expected_dy = std::get<3>(expected_shift);
        const float shift_magnitude = sqrt(float(expected_dx*expected_dx + expected_dy*expected_dy));
        if (shift_magnitude > max_shift_magnitude) {
            max_shift_magnitude = shift_magnitude;
        }
    }

    ConfigurableAlgorithmSettingsMap configuration_map;
    configuration_map.numerical_settings["Maximal allowed shift in pixels"] = 1.5f * max_shift_magnitude;
    configuration_map.numerical_settings["Number of features to detect"] = 8000;

    ReferencePhotoHandlerSurface reference_photo_handler(reference_frame, configuration_map);

    unique_ptr<AlignmentResultBase> plate_solving_result = reference_photo_handler.calculate_alignment(alternative_frame);

    for (const std::tuple<int,int,int,int> &expected_shift : expected_shifts) {
        const int x = std::get<0>(expected_shift);
        const int y = std::get<1>(expected_shift);
        const int expected_dx = std::get<2>(expected_shift);
        const int expected_dy = std::get<3>(expected_shift);


        float shifted_x(x), shifted_y(y);
        plate_solving_result->transform_from_reference_to_shifted_frame(&shifted_x, &shifted_y);
        const int dx = shifted_x - x;
        const int dy = shifted_y - y;

        if (dx != expected_dx || dy != expected_dy) {
            error_message += "Alignment point box at (" + to_string(x) + ", " + to_string(y) + ") has incorrect shift: expected (" + to_string(expected_dx) + ", " + to_string(expected_dy) + "), got (" + to_string(dx) + ", " + to_string(dy) + ").\n";
        }
    }

    if (error_message.empty()) {
        return TestResult(true, "Alignment point box grid test passed.");
    } else {
        return TestResult(false, error_message);
    }
};
