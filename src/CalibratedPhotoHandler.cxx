#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/ColorInterpolationTool.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(const std::string &raw_file_address, bool use_color_interpolation)    {
    m_is_raw_file = is_raw_file(raw_file_address);
    if (m_is_raw_file)  {
        m_data_original = read_raw_file<short>(raw_file_address, &m_width, &m_height, &m_colors_original);
        m_color_conversion_table = get_color_info_as_number(raw_file_address);
        m_input_file_type = InputFileType::RAW_RGB;
    }
    else {
        m_data_original_color_interpolation = read_rgb_image<short>(raw_file_address, &m_width, &m_height);
        m_input_file_type = InputFileType::IMAGE_RGB;
    }
    m_use_color_interpolation = use_color_interpolation;

    m_y_min = 0;
    m_y_max = m_height;
};

void CalibratedPhotoHandler::define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation)   {
    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation);
};

void CalibratedPhotoHandler::define_local_shifts(const LocalShiftsHandler &local_shifts_handler)   {
    m_local_shifts_handler = local_shifts_handler;
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
    // firstly apply the calibration frames on the original data
    for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame_handler : m_calibration_frames) {
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                const unsigned int index = y*m_width + x;
                auto &value = m_data_original[index];
                value = calibration_frame_handler->get_updated_pixel_value(value, x, y);
            }
        }
    }

    if (m_use_color_interpolation || !m_is_raw_file) {
        // run color interpolation on original data (before alignment and calibration)
        if (m_use_color_interpolation && m_is_raw_file) {
            run_color_interpolation();
        }

        // having the interpolated values for all pixels, let's just shift them
        for (int color = 0; color < 3; color++) {
            m_data_shifted_color_interpolation.push_back(vector<short int>(m_width*m_height, -1));
            for (int y_shifted = 0; y_shifted < m_height; y_shifted++)  {
                for (int x_shifted = 0; x_shifted < m_width; x_shifted++)   {
                    float x_original = x_shifted;
                    float y_original = y_shifted;
                    // translations and rotations
                    m_geometric_transformer->transform_from_reference_to_shifted_frame(&x_original, &y_original);

                    // seeing effect, accounting for local shifts
                    if (!m_local_shifts_handler.empty()) {
                        int x_int = int(x_original);
                        int y_int = int(y_original);
                        if (m_local_shifts_handler.calculate_shifted_coordinates(x_int, y_int, &x_int, &y_int)) {
                            x_original = x_int;
                            y_original = y_int;
                        }
                        else {
                            continue;
                        }
                    }
                    int x_int = int(x_original);
                    int y_int = int(y_original);
                    if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
                        const unsigned int index_shifted = y_shifted*m_width + x_shifted;
                        const unsigned int index_original = y_int*m_width + x_int;
                        m_data_shifted_color_interpolation[color][index_shifted] = m_data_original_color_interpolation[color][index_original];
                    }
                }
            }
        }
    }
    else {  // do not use color interpolation
        m_data_shifted   = vector<short int>(m_width*m_height, -1);
        m_colors_shifted = vector<char>(m_width*m_height);

        for (int y_shifted = 0; y_shifted < m_height; y_shifted++)  {
            for (int x_shifted = 0; x_shifted < m_width; x_shifted++)   {
                float x_original = x_shifted;
                float y_original = y_shifted;
                // translations and rotations
                m_geometric_transformer->transform_from_reference_to_shifted_frame(&x_original, &y_original);

                // seeing effect, accounting for local shifts
                if (!m_local_shifts_handler.empty()) {
                    int x_int = int(x_original);
                    int y_int = int(y_original);
                    if (m_local_shifts_handler.calculate_shifted_coordinates(x_int, y_int, &x_int, &y_int)) {
                        x_original = x_int;
                        y_original = y_int;
                    }
                    else {
                        continue;
                    }
                }
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

                    m_data_shifted[index_shifted] = m_data_original[index_original];
                    m_colors_shifted[index_shifted] = m_colors_original[index_original];
                }
            }
        }
    }

    // clean up unused memory
    m_data_original.clear();
    m_colors_original.clear();
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(int x, int y, short int *value, char *color) const {
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

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(int x, int y, int color, short int *value) const   {
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
    ColorInterpolationTool color_interpolation_tool(m_data_original.data(), m_width, m_height, m_colors_original, m_color_conversion_table);
    m_data_original_color_interpolation = color_interpolation_tool.get_interpolated_rgb_image();
};
