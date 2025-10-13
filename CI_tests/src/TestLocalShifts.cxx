
#include "../headers/TestLocalShifts.h"

#include "../../headers/PixelType.h"
#include "../../headers/InputFrameReader.h"
#include "../../headers/AlignmentPointBoxGrid.h"
#include "../../headers/AlignmentWindow.h"
#include "../../headers/MonochromeImageData.h"
#include "../../headers/Common.h"
#include "../../headers/GaussianBlur.h"

#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

TestResult AstroPhotoStacker::test_predefined_alignment_boxes(  const InputFrame &reference_frame,
                                                                const InputFrame &alternative_frame,
                                                                const std::vector<std::tuple<int,int,int,int>> &alignment_points,
                                                                const std::vector<std::pair<int,int>> &expected_shifts) {

    string error_message;

    InputFrameReader reference_frame_data(reference_frame);
    reference_frame_data.load_input_frame_data();
    InputFrameReader alternative_frame_data(alternative_frame);
    alternative_frame_data.load_input_frame_data();

    const int width_reference = reference_frame_data.get_width();
    const int height_reference = reference_frame_data.get_height();
    const vector<vector<PixelType>> reference_image_data_rgb = reference_frame_data.get_rgb_data();
    const vector<PixelType> reference_image_data_monochrome = convert_color_to_monochrome(reference_image_data_rgb, width_reference, height_reference);
    MonochromeImageData image_data_reference;
    image_data_reference.brightness = reference_image_data_monochrome.data();
    image_data_reference.width = width_reference;
    image_data_reference.height = height_reference;
    const MonochromeImageDataWithStorage blurred_image_reference = gaussian_blur(image_data_reference, 5, 5, 1);

    const AlignmentWindow alignment_window{0,0,width_reference-1, height_reference-1};

    AlignmentPointBoxGrid alignment_point_box_grid(
        blurred_image_reference,
        alignment_window,
        alignment_points,
        false // do not sort alignment points - this would make it harder to compare
    );


    const int width_alternative = alternative_frame_data.get_width();
    const int height_alternative = alternative_frame_data.get_height();
    const vector<vector<PixelType>> alternative_image_data_rgb = alternative_frame_data.get_rgb_data();
    const vector<PixelType> alternative_image_data_monochrome = convert_color_to_monochrome(alternative_image_data_rgb, width_alternative, height_alternative);
    MonochromeImageData image_data_alternative;
    image_data_alternative.brightness = alternative_image_data_monochrome.data();
    image_data_alternative.width = width_alternative;
    image_data_alternative.height = height_alternative;


    const MonochromeImageDataWithStorage blurred_image_alternative = gaussian_blur(image_data_alternative, 5, 5, 1);


    const vector<LocalShift> local_shifts = alignment_point_box_grid.get_local_shifts(blurred_image_alternative);
    if (local_shifts.size() != expected_shifts.size()) {
        error_message += "Number of alignment point boxes does not match the expected number.\n";
    }
    for (unsigned int i_box = 0; i_box < local_shifts.size(); i_box++) {
        const LocalShift &local_shift = local_shifts[i_box];
        const int x = local_shift.x;
        const int y = local_shift.y;
        const int shift_x = local_shift.dx;
        const int shift_y = local_shift.dy;

        if (x < 0 || x >= width_reference || y < 0 || y >= height_reference) {
            error_message += "Alignment point box " + to_string(i_box) + " is out of bounds.\n";
            continue;
        }

        if (shift_x != expected_shifts[i_box].first || shift_y != expected_shifts[i_box].second) {
            error_message += "Alignment point box " + to_string(i_box) + " has incorrect shift: expected (" + to_string(expected_shifts[i_box].first) + ", " + to_string(expected_shifts[i_box].second) + "), got (" + to_string(shift_x) + ", " + to_string(shift_y) + ").\n";
        }
    }


    if (error_message.empty()) {
        return TestResult(true, "Alignment point box grid test passed.");
    } else {
        return TestResult(false, error_message);
    }
};
