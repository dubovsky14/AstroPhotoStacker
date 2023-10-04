#include "../headers/FlatFrameHandler.h"

#include "../headers/raw_file_reader.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace AstroPhotoStacker;

FlatFrameHandler::FlatFrameHandler(const std::string &input_file) {
    try    {
        // raw file
        m_data_original = read_raw_file<unsigned short int>(input_file, &m_width, &m_height, &m_colors);
    }
    catch(const std::exception& e)    {
        try        {
            // picture file
            cv::Mat image = cv::imread(input_file, cv::IMREAD_GRAYSCALE);
            m_width = image.cols;
            m_height = image.rows;
            m_data_original = make_unique<unsigned short int[]>(m_width*m_height);
            for (int y = 0; y < m_height; y++) {
                for (int x = 0; x < m_width; x++) {
                    m_data_original[y*m_width + x] = image.at<uchar>(y, x);
                }
            }
            m_colors = vector<char>(m_width*m_height, 0);
        }
        catch(const std::exception& e)        {
            throw e;
        }
    }
    calibrate();
};

float FlatFrameHandler::get_pixel_value(int x, int y) const {
    return m_data_calibrated[y*m_width + x];
};

void FlatFrameHandler::calibrate() {
    if (m_data_original == nullptr && m_data_calibrated.size() != 0) {
        return;
    }

    m_data_calibrated = vector<float>(m_width*m_height);

    // get maximum values
    unsigned short int max_values[3] = {0, 0, 0};
    for (int i = 0; i < m_width*m_height; i++) {
        if (m_data_original[i] > max_values[i]) {
            max_values[i] = m_data_original[i];
        }
    }

    float inverted_max_values[3] = {1.0f/max_values[0], 1.0f/max_values[1], 1.0f/max_values[2]};

    // normalize
    for (int i = 0; i < m_width*m_height; i++) {
        m_data_calibrated[i] = m_data_original[i] * inverted_max_values[m_colors[i]];
    }

    m_data_original = nullptr;
    m_colors.clear();
}
