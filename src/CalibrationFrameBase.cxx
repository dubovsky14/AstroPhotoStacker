#include "../headers/CalibrationFrameBase.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include <opencv2/opencv.hpp>

#include <string>

using namespace AstroPhotoStacker;
using namespace std;


CalibrationFrameBase::CalibrationFrameBase(const InputFrame &input_frame) {
    const std::string input_file = input_frame.get_file_address();

    if (input_frame.is_video_frame()) {
        throw runtime_error("CalibrationFrameBase::CalibrationFrameBase: video frames are not supported");
    }

    try    {
        // raw file
        m_data_original = read_raw_file<unsigned short int>(input_frame, &m_width, &m_height, &m_colors);
    }
    catch(const std::exception& e)    {
        try        {
            m_data_original = read_rgb_image_as_gray_scale<unsigned short int>(input_frame, &m_width, &m_height);
            m_colors = vector<char>(m_width*m_height, 0);
        }
        catch(const std::exception& e)        {
            throw e;
        }
    }
};

CalibrationFrameBase::CalibrationFrameBase(int width, int height, const std::vector<double> &image)   {
    m_width = width;
    m_height = height;
    m_data_original = vector<unsigned short int>(m_width*m_height);
    for (int i = 0; i < m_width*m_height; i++) {
        m_data_original[i] = image[i];
    }
    m_colors = vector<char>(m_width*m_height, 0);
};
