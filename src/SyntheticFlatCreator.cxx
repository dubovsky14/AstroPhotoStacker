#include "../headers/SyntheticFlatCreator.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/KDTree.h"

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

SyntheticFlatCreator::SyntheticFlatCreator(const std::string &input_file) {
    load_data(input_file);
};

void SyntheticFlatCreator::create_and_save_synthetic_flat(const std::string &output_file) {
    calculate_threshold();
    replace_values_above_threshold();
    save_flat(output_file);
};

void SyntheticFlatCreator::load_data(const std::string &input_file) {
    CalibratedPhotoHandler calibrated_photo_handler(input_file, true);
    calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
    calibrated_photo_handler.calibrate();

    m_height = calibrated_photo_handler.get_height();
    m_width = calibrated_photo_handler.get_width();

    vector<vector<short unsigned int>> calibrated_color_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

    m_original_gray_scale_data.resize(calibrated_color_data[0].size(), 0);
    for (unsigned int i = 0; i < calibrated_color_data[0].size(); i++) {
        int mean = 0;    // ushort is 16 bits, so it could overflow
        for (unsigned int i_color = 0; i_color < calibrated_color_data.size(); i_color++) {
            mean += calibrated_color_data[i_color][i];
        }
        mean /= calibrated_color_data.size();
        m_original_gray_scale_data[i] = mean;
    }
}

void SyntheticFlatCreator::calculate_threshold()    {
    vector<int> histogram(1 << 16, 0);
    for (unsigned short pixel_value : m_original_gray_scale_data) {
        histogram[pixel_value]++;
    }

    const float fraction = 0.6;
    const int total_pixels = m_width * m_height;
    int sum = 0;
    for (unsigned short pixel_value = 0; pixel_value < histogram.size(); pixel_value++) {
        sum += histogram[pixel_value];
        if (sum > total_pixels*fraction) {
            m_threshold = pixel_value;
            break;
        }
    }
};

void SyntheticFlatCreator::replace_values_above_threshold() {

};


void SyntheticFlatCreator::get_flat_center(int *center_x, int *center_y)    {

};

void SyntheticFlatCreator::save_flat(const std::string &output_file)    {

};