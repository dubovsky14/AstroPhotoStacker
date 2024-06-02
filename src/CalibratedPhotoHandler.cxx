#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ColorInterpolationTool.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(const std::string &raw_file_address, bool use_color_interpolation)    {
    m_data_original = read_raw_file<unsigned short>(raw_file_address, &m_width, &m_height, &m_colors_original);
    m_color_conversion_table = get_color_info_as_number(raw_file_address);
    m_use_color_interpolation = use_color_interpolation;

    m_y_min = 0;
    m_y_max = m_height;
};

void CalibratedPhotoHandler::define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation)   {
    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation);
};

void CalibratedPhotoHandler::limit_y_range(int y_min, int y_max) {
    if (y_min >= y_max) {
        return;
    }

    if (y_min >= 0 || y_min < m_height) {
        y_min = y_min;
    }

    if (y_max >= 0 || y_max < m_height) {
        y_max = y_max;
    }
};

void CalibratedPhotoHandler::register_calibration_frame(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler)  {
    m_calibration_frames.push_back(std::move(calibration_frame_handler));
};

void CalibratedPhotoHandler::register_hot_pixel_identifier(const HotPixelIdentifier *hot_pixel_identifier)  {
    m_hot_pixel_identifier = hot_pixel_identifier;
};

void CalibratedPhotoHandler::set_bit_depth(unsigned short int bit_depth)    {
    m_max_allowed_pixel_value = 1 << bit_depth;
};

void CalibratedPhotoHandler::calibrate() {
    if (m_use_color_interpolation) {
        // run color interpolation on original data (before alignment and calibration)
        run_color_interpolation();

        // having the interpolated values for all pixels, let's just shift them
        for (int color = 0; color < 3; color++) {
            m_data_shifted_color_interpolation.push_back(vector<unsigned short int>(m_width*m_height, 0));
            for (int y_shifted = 0; y_shifted < m_height; y_shifted++)  {
                for (int x_shifted = 0; x_shifted < m_width; x_shifted++)   {
                    float x_original = x_shifted;
                    float y_original = y_shifted;
                    m_geometric_transformer->transform_from_reference_to_shifted_frame(&x_original, &y_original);
                    int x_int = int(x_original);
                    int y_int = int(y_original);
                    if (m_hot_pixel_identifier != nullptr) {
                        if (m_hot_pixel_identifier->is_hot_pixel(x_int, y_int)) {
                            fix_hot_pixel(x_int, y_int);
                        }
                    }
                    if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
                        const unsigned int index_shifted = y_shifted*m_width + x_shifted;
                        const unsigned int index_original = y_int*m_width + x_int;
                        for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame_handler : m_calibration_frames) {
                            auto &value = m_data_original_color_interpolation[color][index_original];
                            value = calibration_frame_handler->get_updated_pixel_value(value, x_int, y_int);
                        }

                        m_data_shifted_color_interpolation[color][index_shifted] = m_data_original_color_interpolation[color][index_original];
                    }
                }
            }
        }
    }
    else {  // do not use color interpolation
        m_data_shifted   = vector<unsigned short int>(m_width*m_height);
        m_colors_shifted = vector<char>(m_width*m_height, -1);

        for (int y_shifted = 0; y_shifted < m_height; y_shifted++)  {
            for (int x_shifted = 0; x_shifted < m_width; x_shifted++)   {
                float x_original = x_shifted;
                float y_original = y_shifted;
                m_geometric_transformer->transform_from_reference_to_shifted_frame(&x_original, &y_original);
                int x_int = int(x_original);
                int y_int = int(y_original);
                if (m_hot_pixel_identifier != nullptr) {
                    if (m_hot_pixel_identifier->is_hot_pixel(x_int, y_int)) {
                        fix_hot_pixel(x_int, y_int);
                    }
                }
                if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
                    const unsigned int index_shifted = y_shifted*m_width + x_shifted;
                    const unsigned int index_original = y_int*m_width + x_int;

                    for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame_handler : m_calibration_frames) {
                        auto &value = m_data_original[index_original];
                        value = calibration_frame_handler->get_updated_pixel_value(value, x_int, y_int);
                    }

                    m_data_shifted[index_shifted] = m_data_original[index_original];
                    m_colors_shifted[index_shifted] = m_colors_original[index_original];
                }
            }
        }
    }

    // clean up unused memory
    m_data_original = nullptr;
    m_colors_original.clear();
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(int x, int y, unsigned int *value, char *color) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        const unsigned int index = y*m_width + x;
        *value = m_data_shifted[index];
        *color = m_color_conversion_table[m_colors_shifted[index]];
    }
    else {
        *value = 0;
        *color = -1;
    }
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(int x, int y, int color, unsigned int *value) const   {
    if (!m_use_color_interpolation) {
        *value = 0;
        return;
    }

    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        const unsigned int index = y*m_width + x;
        *value = m_data_shifted_color_interpolation[color][index];
    }
    else {
        *value = 0;
    }
};

void CalibratedPhotoHandler::fix_hot_pixel(int x, int y)    {
    int n_same_color_neighbors = 0;
    int new_value = 0;
    const int index = y*m_width + x;
    const int color_this_pixel = m_colors_original[index];
    for (int shift_size = 1; shift_size <= 2; shift_size++) {
        for (int i_shift_y = -1*shift_size; i_shift_y <= shift_size; i_shift_y++) {
            const int neighbor_y = y + i_shift_y;
            if (neighbor_y < 0 || neighbor_y >= m_height) {
                continue;
            }
            for (int i_shift_x = -1*shift_size; i_shift_x <= shift_size; i_shift_x++) {
                if (i_shift_x == 0 && i_shift_y == 0) {
                    continue;
                }
                const int neighbor_x = x + i_shift_x;
                if (neighbor_x < 0 || neighbor_x >= m_width) {
                    continue;
                }
                const int neighbor_index = neighbor_y*m_width + neighbor_x;
                if (m_colors_original[neighbor_index] == color_this_pixel) {
                    n_same_color_neighbors++;
                    new_value += m_data_original[neighbor_index];
                }
            }
        }
        if (n_same_color_neighbors != 0) {
            new_value /= n_same_color_neighbors;
            m_data_original[index] = new_value;
            return;
        }
    }
    cout << "Unable to fix hot pixel: " + to_string(x) + " " + to_string(y) + "\n";
};

void CalibratedPhotoHandler::run_color_interpolation()  {
    ColorInterpolationTool color_interpolation_tool(m_data_original.get(), m_width, m_height, m_colors_original, m_color_conversion_table);
    m_data_original_color_interpolation = color_interpolation_tool.get_interpolated_rgb_image();
};
