#include "../headers/CalibrationFrameBase.h"
#include "../headers/raw_file_reader.h"
#include <opencv2/opencv.hpp>

#include <string>

using namespace AstroPhotoStacker;
using namespace std;


CalibrationFrameBase::CalibrationFrameBase(const std::string &input_file) {
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
};

CalibrationFrameBase::CalibrationFrameBase(const CalibrationFrameBase &other) {
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
