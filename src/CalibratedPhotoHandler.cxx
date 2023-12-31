#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/raw_file_reader.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(const std::string &raw_file_address)    {
    m_data_original = read_raw_file<unsigned short>(raw_file_address, &m_width, &m_height, &m_colors_original);
    m_color_conversion_table = get_color_info_as_number(raw_file_address);

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

void CalibratedPhotoHandler::register_flat_frame(const FlatFrameHandler *flat_frame_handler)   {
    m_flat_frame = flat_frame_handler;
};

void CalibratedPhotoHandler::register_hot_pixel_identifier(const HotPixelIdentifier *hot_pixel_identifier)  {
    m_hot_pixel_identifier = hot_pixel_identifier;
};

void CalibratedPhotoHandler::set_bit_depth(unsigned short int bit_depth)    {
    m_max_allowed_pixel_value = 1 << bit_depth;
};

void CalibratedPhotoHandler::calibrate() {
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
                if (m_flat_frame == nullptr) {
                    m_data_shifted[index_shifted] = m_data_original[index_original];
                }
                else {
                    const float value = m_data_original[index_original]*m_flat_frame->get_pixel_value_inverted(x_int, y_int);
                    m_data_shifted[index_shifted] = min(m_max_allowed_pixel_value, (unsigned int)(value)); // stars in corners can "overflow" without this check
                }
                m_colors_shifted[index_shifted] = m_colors_original[index_original];
            }
        }
    }

    m_data_original = nullptr;
    m_colors_original.clear();
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const {
    const int x_int = int(x);
    const int y_int = int(y);

    if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
        const unsigned int index = y_int*m_width + x_int;
        *value = m_data_shifted[index];
        *color = m_color_conversion_table[m_colors_shifted[index]];
    }
    else {
        *value = 0;
        *color = -1;
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
