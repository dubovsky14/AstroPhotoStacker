#include "../headers/FlatFrameHandler.h"

#include <opencv2/opencv.hpp>

#include <map>

using namespace std;
using namespace AstroPhotoStacker;

FlatFrameHandler::FlatFrameHandler(const InputFrame &input_frame) : CalibrationFrameBase(input_frame) {
    calibrate();
};

FlatFrameHandler::FlatFrameHandler(const FlatFrameHandler &other) : CalibrationFrameBase(other) {
    calibrate();
};

FlatFrameHandler::FlatFrameHandler(int width, int height, const std::vector<double> &image) : CalibrationFrameBase(width, height, image) {
    calibrate();
};

void FlatFrameHandler::calibrate() {
    if (m_data_original.size() == 0 && m_data_calibrated.size() != 0) {
        return;
    }

    m_data_calibrated = vector<float>(m_width*m_height);

    // get maximum values
    PixelType max_values[3] = {1, 1, 1};   // to avoid division by zero if not initialized
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            const int index = y*m_width + x;
            const int bayer_index = (y % 2) * 2 + x % 2;
            if (m_data_original[index] > max_values[int(m_colors[bayer_index])]) {
                max_values[int(m_colors[bayer_index])] = m_data_original[index];
            }
        }
    }

    // normalize
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            const int index = y*m_width + x;
            const int bayer_index = (y % 2) * 2 + x % 2;
            m_data_calibrated[index] = double(max_values[int(m_colors[bayer_index])]) / m_data_original[index];
        }
    }

    // smooth it because of the bayer filter/fluctuations:
    for (int y = 0; y < m_height-1; y++) {
        for (int x = 0; x < m_width-1; x++) {
            m_data_calibrated[y*m_width + x] = (m_data_calibrated[y*m_width + x] + m_data_calibrated[y*m_width + x+1] + m_data_calibrated[(y+1)*m_width + x] + m_data_calibrated[(y+1)*m_width + x+1]) / 4.0;
        }
    }

    // now the borders
    for (int y = 0; y < m_height; y++) {
        m_data_calibrated[y*m_width + m_width-1] = m_data_calibrated[y*m_width + m_width-2];
    }

    for (int x = 0; x < m_width; x++) {
        m_data_calibrated[(m_height-1)*m_width + x] = m_data_calibrated[(m_height-2)*m_width + x];
    }

    m_data_original.clear();
}
