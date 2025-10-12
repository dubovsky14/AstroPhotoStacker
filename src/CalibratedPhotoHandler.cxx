#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/Debayring.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(const InputFrame &input_frame, bool use_color_interpolation)    {
    m_use_color_interpolation = use_color_interpolation;
    m_input_frame_data_original = make_unique<InputFrameReader>(input_frame);
    m_input_frame_data_original->load_input_frame_data();
    m_input_frame_data_original->get_photo_resolution(&m_width, &m_height);

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
    if (!m_local_shifts_handler.empty()) {
        m_score_handler.initialize_local_scores(m_width, m_height, 1);
    }

    vector<std::vector<PixelType>*> data_for_calibration = m_input_frame_data_original->get_all_data_for_calibration();
    // firstly apply the calibration frames on the original data
    for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame_handler : m_calibration_frames) {
        for (std::vector<PixelType>* data : data_for_calibration) {
            calibration_frame_handler->apply_calibration(data);
        }
    }

    // have to fix hot pixels before debayering
    if (m_hot_pixel_identifier != nullptr && m_input_frame_data_original->is_raw_file_before_debayering()) {
        vector<PixelType>& raw_data = m_input_frame_data_original->get_raw_data_non_const();
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                if (m_hot_pixel_identifier->is_hot_pixel(x, y)) {
                    fix_hot_pixel(x, y, &raw_data);
                }
            }
        }
    }

    if (m_use_color_interpolation && m_input_frame_data_original->is_raw_file()) {
        m_input_frame_data_original->debayer();
    }

    // having the interpolated values for all pixels, let's just shift them
    for (int color = 0; color < 3; color++) {
        m_data_shifted_color_interpolation.push_back(vector<PixelType>(m_width*m_height, -1));
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
                    float score = 1;
                    if (m_local_shifts_handler.calculate_shifted_coordinates(x_int, y_int, &x_int, &y_int, &score)) {
                        x_original = x_int;
                        y_original = y_int;
                        m_score_handler.set_local_score(x_shifted, y_shifted, score);
                    }
                    else {
                        continue;
                    }
                }
                int x_int = int(x_original);
                int y_int = int(y_original);
                if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
                    const unsigned int index_shifted = y_shifted*m_width + x_shifted;
                    m_data_shifted_color_interpolation[color][index_shifted] = m_input_frame_data_original->get_pixel_value(x_int, y_int, color);
                }
            }
        }
    }

    // clean up unused memory
    m_input_frame_data_original = nullptr;
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(int x, int y, short int *value, char *color) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        const unsigned int index = y*m_width + x;
        *value = m_data_shifted[index];
        *color = m_colors_shifted[index];
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


void CalibratedPhotoHandler::fix_hot_pixel(int x, int y, std::vector<PixelType> *data)    {
    int n_same_color_neighbors = 0;
    int new_value = 0;
    const int index = y*m_width + x;
    const int color_this_pixel = m_input_frame_data_original->get_raw_color(x,y);
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
                if (m_input_frame_data_original->get_raw_color(neighbor_x, neighbor_y) == color_this_pixel) {
                    n_same_color_neighbors++;
                    new_value += data->at(neighbor_index);
                }
            }
        }
        if (n_same_color_neighbors != 0) {
            new_value /= n_same_color_neighbors;
            data->at(index) = new_value;
            return;
        }
    }
};
