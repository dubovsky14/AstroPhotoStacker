#include "../headers/TestUtils.h"
#include "../headers/ImageReadingTest.h"

#include "../../headers/InputFrameData.h"

#include <string>
#include <vector>
#include <tuple>

class InputFrame;


using namespace AstroPhotoStacker;
using namespace std;

TestResult AstroPhotoStacker::test_image_reading_raw(   const InputFrame &input_frame,
                                                        const std::pair<int,int> &expected_resolution,
                                                        const std::vector<std::tuple<int, int, short int, char>> &expected_pixel_values)   {

    InputFrameData<short int> input_frame_data(input_frame);

    std::string error_message = "";
    if (input_frame_data.get_width() != expected_resolution.first) {
        error_message += "Width mismatch: expected " + std::to_string(expected_resolution.first) + ", got " + std::to_string(input_frame_data.get_width()) + "\n";
    }
    if (input_frame_data.get_height() != expected_resolution.second) {
        error_message += "Height mismatch: expected " + std::to_string(expected_resolution.second) + ", got " + std::to_string(input_frame_data.get_height()) + "\n";
    }
    for (const auto &pixel : expected_pixel_values) {
        const int x = std::get<0>(pixel);
        const int y = std::get<1>(pixel);
        const short int expected_value = std::get<2>(pixel);
        const char expected_color = std::get<3>(pixel);

        const short int actual_value = input_frame_data.get_pixel_value_raw(x, y);
        if (actual_value != expected_value) {
            error_message += "Pixel value mismatch at (" + std::to_string(x) + ", " + std::to_string(y) + "): expected " + std::to_string(expected_value) + ", got " + std::to_string(actual_value) + "\n";
        }

        const char actual_color = input_frame_data.get_color(x, y);
        if (actual_color != expected_color) {
            error_message += "Color mismatch at (" + std::to_string(x) + ", " + std::to_string(y) + "): expected " + std::to_string(expected_color) + ", got " + std::to_string(actual_color) + "\n";
        }
    }

    if (error_message.empty()) {
        return TestResult(true, "Image reading test passed.");
    } else {
        return TestResult(false, error_message);
    }
};
