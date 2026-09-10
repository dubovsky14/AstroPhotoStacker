#include "../headers/LensCorrectionTool.h"
#include "../headers/Common.h"

#include <vector>
#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;

LensCorrectionTool::LensCorrectionTool(const std::string &initialization_string) {
    // Parse the initialization string and set the member variables accordingly
    // Example format: "c_x=2026;c_y=3177;k1=0.000903978;k2=-0.000104815;k3=0;sensor_half_diagonal_squared=1.41358e+07"

    const std::vector<std::string> key_value_pairs = split_string(initialization_string, ";");

    for (const std::string &key_value_pair : key_value_pairs) {
        const std::vector<std::string> key_and_value = split_string(key_value_pair, "=");
        if (key_and_value.size() != 2) continue;
        const std::string &key = key_and_value[0];
        const std::string &value = key_and_value[1];

        if (!string_is_float(value)) {
            throw std::invalid_argument("Value is not a valid float: " + value);
        }

        if (key == "c_x") c_x = std::stof(value);
        else if (key == "c_y") c_y = std::stof(value);
        else if (key == "k1") k1 = std::stof(value);
        else if (key == "k2") k2 = std::stof(value);
        else if (key == "k3") k3 = std::stof(value);
        else if (key == "sensor_half_diagonal_squared") sensor_half_diagonal_squared = std::stof(value);
        else {
            throw std::invalid_argument("Unknown key in initialization string: " + key);
        }
    }
};

void LensCorrectionTool::transform_from_sensor_to_undistorted(float *x, float *y) const {
    const double x_distorted = *x - c_x;
    const double y_distorted = *y - c_y;

    const double r2 = (x_distorted * x_distorted + y_distorted * y_distorted) / sensor_half_diagonal_squared;
    const double r4 = r2 * r2;
    const double r6 = r4 * r2;

    const double radial_distortion = 1 + k1 * r2 + k2 * r4 + k3 * r6;

    const double x_undistorted = x_distorted * radial_distortion + c_x;
    const double y_undistorted = y_distorted * radial_distortion + c_y;

    *x = static_cast<float>(x_undistorted);
    *y = static_cast<float>(y_undistorted);
};
