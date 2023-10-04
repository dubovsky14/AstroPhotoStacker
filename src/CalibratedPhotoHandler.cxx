#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/raw_file_reader.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(const std::string &raw_file_address)    {
    m_data_original = read_raw_file<unsigned short>(raw_file_address, &m_width, &m_height, &m_colors_original);
    m_color_conversion_table = get_color_info_as_number(raw_file_address);
};

void CalibratedPhotoHandler::define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation)   {
    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation);
};

void CalibratedPhotoHandler::apply_flat_frame(const FlatFrameHandler &flat_frame_handler)   {
    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            m_data_original[y*m_width + x] *= flat_frame_handler.get_pixel_value(x, y);
        }
    }
};

void CalibratedPhotoHandler::calibrate() {
    m_data_shifted   = vector<unsigned short int>(m_width*m_height);
    m_colors_shifted = vector<char>(m_width*m_height, -1);

    for (int y = 0; y < m_height; y++)  {
        for (int x = 0; x < m_width; x++)   {
            float x_float = x;
            float y_float = y;
            m_geometric_transformer->transform_to_reference_frame(&x_float, &y_float);
            int x_int = int(x_float);
            int y_int = int(y_float);
            if (x_int >= 0 && x_int < m_width && y_int >= 0 && y_int < m_height) {
                const unsigned int index_shifted = y_int*m_width + x_int;
                const unsigned int index_original = y*m_width + x;
                m_data_shifted[index_shifted]   = m_data_original[index_original];
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
