#pragma once

#include <utility>
#include <string>
#include <vector>


namespace AstroPhotoStacker {
    struct LensCorrectionTool    {
        LensCorrectionTool() = default;

        LensCorrectionTool(const std::string &initialization_string);

        float c_x = 0; // x-coordinate of lens' optical axis in pixels
        float c_y = 0; // y-coordinate of lens' optical axis in pixels
        float k1 = 0; // Term associated to r^2
        float k2 = 0; // Term associated to r^4
        float k3 = 0; // Term associated to r^6

        float sensor_half_diagonal_squared = 0; // Sensor half diagonal in pixels

        void transform_from_sensor_to_undistorted(float *x, float *y)   const;

        template<typename ValueType>
        std::vector<ValueType> get_undistorted_image(const std::vector<ValueType> &distorted_image, int width, int height) const {
            std::vector<ValueType> undistorted_image(distorted_image.size(), -1);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const int index_sensor = y * width + x;
                    float fx = static_cast<float>(x);
                    float fy = static_cast<float>(y);
                    transform_from_sensor_to_undistorted(&fx, &fy);
                    if (fx < 0 || fx >= width || fy < 0 || fy >= height) continue;
                    const int index_undistorted = static_cast<int>(fy) * width + static_cast<int>(fx);
                    undistorted_image.at(index_undistorted) = distorted_image.at(index_sensor);

                }
            }
            return undistorted_image;
        };
    };
}