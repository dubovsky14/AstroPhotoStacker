#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/GaussianBlur.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace AstroPhotoStacker;
using namespace std;


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const InputFrame &input_frame, float threshold_fraction) : ReferencePhotoHandlerPlanetary(input_frame, threshold_fraction) {
    m_threshold_fraction = threshold_fraction;
    CalibratedPhotoHandler calibrated_photo_handler(input_frame, true);
    calibrated_photo_handler.define_alignment(0,0,0,0,0);
    calibrated_photo_handler.calibrate();

    m_width  = calibrated_photo_handler.get_width();
    m_height = calibrated_photo_handler.get_height();
    const vector<vector<short int>> &calibrated_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

    vector<unsigned short int> brightness(m_width*m_height);
    for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
        float value = 0;
        int n_points = 0;
        for (unsigned int color = 0; color < calibrated_data.size(); color++) {
            if (calibrated_data[color][i_pixel] >= 0) {
                value += calibrated_data[color][i_pixel];
                n_points++;
            }
        }
        value /= n_points;
        brightness.at(i_pixel) = value;
    }

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

    const int box_size = 50;
    const int box_spacing = 5;

    m_alignment_point_box_grid = make_unique<AlignmentPointBoxGrid>(blurred_image_data, m_alignment_window, box_size, box_spacing, m_center_of_mass_x, m_center_of_mass_y);

    cout << "Alignment grid initialized, number of boxes: " << m_alignment_point_box_grid->get_alignment_boxes().size() << endl;
};

std::vector<LocalShift> ReferencePhotoHandlerSurface::get_local_shifts( const InputFrame &input_frame,
                                                                        float shift_x,
                                                                        float shift_y,
                                                                        float rotation_center_x,
                                                                        float rotation_center_y,
                                                                        float rotation) const   {

    CalibratedPhotoHandler calibrated_photo_handler(input_frame, true);
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
        brightness.at(i_pixel) = value;
    }

    MonochromeImageData calibrated_image_data;
    calibrated_image_data.brightness = brightness.data();
    calibrated_image_data.width = width;
    calibrated_image_data.height = height;

    const vector<unsigned short int> smeared_data = gaussian_blur(calibrated_image_data, m_blur_window_size, m_blur_window_size, m_blur_sigma);
    MonochromeImageData blurred_image;

    blurred_image.brightness = smeared_data.data();
    blurred_image.width = width;
    blurred_image.height = height;
    return m_alignment_point_box_grid->get_local_shifts(blurred_image);
};