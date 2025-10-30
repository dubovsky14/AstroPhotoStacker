#include "../headers/TestAlignmentResult.h"

#include "../../headers/AlignmentResultFactory.h"
#include "../../headers/AlignmentResultPlateSolving.h"
#include "../../headers/AlignmentResultSurface.h"

#include <map>

using namespace AstroPhotoStacker;
using namespace std;

TestResult AstroPhotoStacker::test_alignment_result_plate_solving() {

    std::unique_ptr<AlignmentResultBase> alignment_result = AlignmentResultFactory::get_instance().create_alignment_result_from_type(AlignmentResultPlateSolving::s_type_name);
    dynamic_cast<AlignmentResultPlateSolving*>(alignment_result.get())->set_parameters(10,15,1000,2300,0.02);

    const std::string description_string = alignment_result->get_description_string();
    std::unique_ptr<AlignmentResultBase> alignment_result_from_description = AlignmentResultFactory::get_instance().create_alignment_result_from_description_string(description_string);

    const std::string description_string_2 = alignment_result_from_description->get_description_string();

    std::string message = "";
    if (description_string != description_string_2) {
        message += "AlignmentResultPlateSolving: Description strings do not match.";
    }

    std::map<std::pair<int,int>, std::pair<int,int>> test_points = {
        {{1000,2300}, {990,2285}},
        {{2000,2800}, {2000,2765}},
    };

    for (const auto &test_point : test_points) {
        float x = test_point.first.first;
        float y = test_point.first.second;
        alignment_result_from_description->transform_from_reference_to_shifted_frame(&x, &y);
        const int expected_x = test_point.second.first;
        const int expected_y = test_point.second.second;
        if (int(round(x)) != expected_x || int(round(y)) != expected_y) {
            message += "AlignmentResultPlateSolving: Transform from reference to shifted frame failed for point (" +
                        to_string(test_point.first.first) + ", " + to_string(test_point.first.second) + "). Expected (" +
                        to_string(expected_x) + ", " + to_string(expected_y) + "), got (" +
                        to_string(int(round(x))) + ", " + to_string(int(round(y))) + "). ";
        }

        alignment_result_from_description->transform_to_reference_frame(&x, &y);
        if (int(round(x)) != test_point.first.first || int(round(y)) != test_point.first.second) {
            message += "AlignmentResultPlateSolving: Transform to reference frame failed for point (" +
                        to_string(expected_x) + ", " + to_string(expected_y) + "). Expected (" +
                        to_string(test_point.first.first) + ", " + to_string(test_point.first.second) + "), got (" +
                        to_string(int(round(x))) + ", " + to_string(int(round(y))) + "). ";
        }
    }

    return TestResult(message.empty(), message);
}