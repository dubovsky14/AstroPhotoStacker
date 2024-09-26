#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/GaussianBlur.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace AstroPhotoStacker;
using namespace std;


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const std::string &raw_file_address, float threshold_fraction) : ReferencePhotoHandlerPlanetary(raw_file_address, threshold_fraction) {
    m_threshold_fraction = threshold_fraction;
    const vector<unsigned short> brightness = read_image_monochrome<unsigned short>(raw_file_address, &m_width, &m_height);
    ReferencePhotoHandlerPlanetary::initialize(brightness.data(), m_width, m_height, threshold_fraction);

    initialize_alignment_grid(brightness.data());
};


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const unsigned short *brightness, int width, int height, float threshold_fraction) : ReferencePhotoHandlerPlanetary(brightness, width, height, threshold_fraction) {
    initialize_alignment_grid(brightness);
};

void ReferencePhotoHandlerSurface::initialize_alignment_grid(const unsigned short *brightness_original) {
    MonochromeImageData image_data;
    image_data.brightness = brightness_original;
    image_data.width = m_width;
    image_data.height = m_height;

    const vector<unsigned short> blurred_brightness = gaussian_blur(image_data, m_blur_window_size, m_blur_window_size, m_blur_sigma);
    MonochromeImageData blurred_image_data;
    blurred_image_data.brightness = blurred_brightness.data();
    blurred_image_data.width = m_width;
    blurred_image_data.height = m_height;

    const int alignment_box_size = get<2>(m_alignment_window) - get<0>(m_alignment_window);
    const int box_size = alignment_box_size / 10;
    const int box_spacing = 0;

    m_alignment_point_box_grid = make_unique<AlignmentPointBoxGrid>(blurred_image_data, m_alignment_window, box_size, box_spacing, m_center_of_mass_x, m_center_of_mass_y);
};

std::vector<std::tuple<int,int,int,int,bool>> ReferencePhotoHandlerSurface::get_local_shifts(   const std::string &file_address,
                                                                                                float shift_x,
                                                                                                float shift_y,
                                                                                                float rotation_center_x,
                                                                                                float rotation_center_y,
                                                                                                float rotation) const   {


    CalibratedPhotoHandler calibrated_photo_handler(file_address, true);
    calibrated_photo_handler.define_alignment(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation);
    calibrated_photo_handler.calibrate();

    const int width  = calibrated_photo_handler.get_width();
    const int height = calibrated_photo_handler.get_height();
    const vector<vector<short int>> &calibrated_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

    vector<unsigned short int> brightness(width*height);
    for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
        float value = 0;
        int n_points = 0;
        for (unsigned int color = 0; color < calibrated_data.size(); color++) {
            if (calibrated_data[color][i_pixel] >= 0) {
                value += calibrated_data[color][i_pixel];
                n_points++;
            }
        }
        value /= n_points;
        brightness[i_pixel] = value;
    }

    MonochromeImageData calibrated_image_data;
    calibrated_image_data.brightness = brightness.data();
    calibrated_image_data.width = width;
    calibrated_image_data.height = height;

    vector<unsigned short int> smeared_data = gaussian_blur(calibrated_image_data, m_blur_window_size, m_blur_window_size, m_blur_sigma);
    calibrated_image_data.brightness = smeared_data.data();

    return m_alignment_point_box_grid->get_local_shifts(calibrated_image_data);
};