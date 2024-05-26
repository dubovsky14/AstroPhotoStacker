#include "../headers/FlatFrameHandler.h"

#include "../headers/raw_file_reader.h"
#include <opencv2/opencv.hpp>

#include <map>

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
            cv::Mat image = cv::imread(input_file, -1);
            m_width = image.cols;
            m_height = image.rows;
            m_data_original = make_unique<unsigned short int[]>(m_width*m_height);
            for (int y = 0; y < m_height; y++) {
                for (int x = 0; x < m_width; x++) {
                    m_data_original[y*m_width + x] = image.at<unsigned short>(y, x);
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

FlatFrameHandler::FlatFrameHandler(const FlatFrameHandler &other) {
    m_width = other.m_width;
    m_height = other.m_height;
    if (other.m_data_original == nullptr) {
        m_data_original = nullptr;
    }
    else {
        m_data_original = make_unique<unsigned short int[]>(m_width*m_height);
        for (int i = 0; i < m_width*m_height; i++) {
            m_data_original[i] = other.m_data_original[i];
        }
    }
    m_data_calibrated = other.m_data_calibrated;
    m_colors = other.m_colors;
};

void FlatFrameHandler::calibrate() {
    if (m_data_original == nullptr && m_data_calibrated.size() != 0) {
        return;
    }

    m_data_calibrated = vector<float>(m_width*m_height);

    // get maximum values
    unsigned short int max_values[3] = {1, 1, 1};   // to avoid division by zero if not initialized
    for (int i = 0; i < m_width*m_height; i++) {
        if (m_data_original[i] > max_values[int(m_colors[i])]) {
            max_values[int(m_colors[i])] = m_data_original[i];
        }
    }

    // normalize
    for (int i = 0; i < m_width*m_height; i++) {
        m_data_calibrated[i] = double(max_values[int(m_colors[i])]) / m_data_original[i];
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

    m_data_original = nullptr;
    m_colors.clear();
}
