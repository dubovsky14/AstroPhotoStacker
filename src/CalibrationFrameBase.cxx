#include "../headers/CalibrationFrameBase.h"
#include "../headers/InputFrameReader.h"
#include "../headers/Common.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <numeric>

using namespace AstroPhotoStacker;
using namespace std;


CalibrationFrameBase::CalibrationFrameBase(const InputFrame &input_frame) {
    InputFrameReader input_frame_reader(input_frame);
    input_frame_reader.load_input_frame_data();
    m_width = input_frame_reader.get_width();
    m_height = input_frame_reader.get_height();

    if (input_frame_reader.is_raw_file()) {
        m_data_original = input_frame_reader.get_raw_data();
        m_colors = input_frame_reader.get_bayer_pattern();
    }
    else {
        m_data_original = input_frame_reader.get_monochrome_data();
        m_colors = std::array<char, 4>{0,0,0,0};
    }
};

CalibrationFrameBase::CalibrationFrameBase(int width, int height, const std::vector<double> &image)   {
    m_width = width;
    m_height = height;
    m_data_original = vector<PixelType>(m_width*m_height);
    for (int i = 0; i < m_width*m_height; i++) {
        m_data_original[i] = image[i];
    }
    m_colors = std::array<char, 4>{0,0,0,0};
};

void CalibrationFrameBase::apply_calibration(std::vector<PixelType> *data) const {
    if (int(data->size()) != m_width*m_height) {
        throw runtime_error("CalibrationFrameBase::apply_calibration: size of the data does not match the size of the calibration frame");
    }

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            const int index = y*m_width + x;
            (*data)[index] = force_range<float>(get_updated_pixel_value((*data)[index], x, y), 0, std::numeric_limits<PixelType>::max());
        }
    }
}