#include "../headers/DarkFrameHandler.h"

#include "../headers/raw_file_reader.h"
#include <opencv2/opencv.hpp>

#include <map>

using namespace std;
using namespace AstroPhotoStacker;

DarkFrameHandler::DarkFrameHandler(const std::string &input_file) : CalibrationFrameBase(input_file) {
    calibrate();
};

DarkFrameHandler::DarkFrameHandler(const DarkFrameHandler &other) : CalibrationFrameBase(other) {
    calibrate();
};

void DarkFrameHandler::calibrate() {
    if (m_data_original == nullptr && m_data_calibrated.size() != 0) {
        return;
    }

    m_data_calibrated = vector<float>(m_width*m_height);

    for (int i = 0; i < m_width*m_height; i++) {
        m_data_calibrated[i] = m_data_original[i];
    }

    m_data_original = nullptr;
    m_colors.clear();
}

float DarkFrameHandler::get_updated_pixel_value(float pixel_value, int x, int y) const {
    return max<float>(0, pixel_value - m_data_calibrated[y*m_width + x]);
};