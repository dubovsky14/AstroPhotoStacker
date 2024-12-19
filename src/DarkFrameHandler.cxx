#include "../headers/DarkFrameHandler.h"

#include <opencv2/opencv.hpp>

#include <map>

using namespace std;
using namespace AstroPhotoStacker;

DarkFrameHandler::DarkFrameHandler(const InputFrame &input_frame) : CalibrationFrameBase(input_frame) {
    calibrate();
};

DarkFrameHandler::DarkFrameHandler(const DarkFrameHandler &other) : CalibrationFrameBase(other) {
    calibrate();
};

DarkFrameHandler::DarkFrameHandler(int width, int height, const std::vector<double> &image) : CalibrationFrameBase(width, height, image) {
    calibrate();
};

void DarkFrameHandler::calibrate() {
    if (m_data_original.size() && m_data_calibrated.size() != 0) {
        return;
    }

    m_data_calibrated = vector<float>(m_width*m_height);

    for (int i = 0; i < m_width*m_height; i++) {
        m_data_calibrated[i] = m_data_original[i];
    }

    m_data_original.clear();
    m_colors.clear();
}

float DarkFrameHandler::get_updated_pixel_value(float pixel_value, int x, int y) const {
    return max<float>(0, pixel_value - m_data_calibrated[y*m_width + x]);
};