
#include "../headers/TestLocalShifts.h"

#include "../../headers/ReferencePhotoHandlerSurface.h"
#include "../../headers/LocalShiftsHandler.h"

#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

TestResult AstroPhotoStacker::test_predefined_alignment_boxes(  const InputFrame &reference_frame,
                                                                const InputFrame &alternative_frame,
                                                                const std::vector<std::tuple<int,int,int,int>> &expected_shifts)    {

    string error_message;

    ReferencePhotoHandlerSurface reference_photo_handler(reference_frame, 0.0005f);

    PlateSolvingResult plate_solving_result = reference_photo_handler.calculate_alignment(alternative_frame);
    const vector<LocalShift> local_shifts = reference_photo_handler.get_local_shifts(alternative_frame);
    LocalShiftsHandler local_shifts_handler(local_shifts);

    for (const std::tuple<int,int,int,int> &expected_shift : expected_shifts) {
        const int x = std::get<0>(expected_shift);
        const int y = std::get<1>(expected_shift);
        const int expected_dx = std::get<2>(expected_shift) + plate_solving_result.shift_x;
        const int expected_dy = std::get<3>(expected_shift) + plate_solving_result.shift_y;

        float shifted_x(x), shifted_y(y);
        bool valid = local_shifts_handler.calculate_shifted_coordinates(&shifted_x, &shifted_y);
        if (!valid) {
            error_message += "Alignment point box at (" + to_string(x) + ", " + to_string(y) + ") is invalid.\n";
            continue;
        }
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
