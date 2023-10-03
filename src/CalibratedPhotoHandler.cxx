#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/raw_file_reader.h"

using namespace std;
using namespace AstroPhotoStacker;

CalibratedPhotoHandler::CalibratedPhotoHandler(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation)    {
    m_geometric_transformer = make_unique<GeometricTransformer>(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation);
};

void CalibratedPhotoHandler::add_raw_data(const std::string &raw_file_address) {
    m_data = read_raw_file<unsigned short>(raw_file_address, &m_width, &m_height, &m_colors);
    m_color_conversion_table = get_color_info_as_number(raw_file_address);
};

void CalibratedPhotoHandler::get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const {
    m_geometric_transformer->transform_from_reference_to_shifted_frame(&x, &y);
    int x_int = int(x);
    int y_int = int(y);

    if (x_int < 0 || x_int >= m_width || y_int < 0 || y_int >= m_height) {
        *color = -1;
        return;
    }
    const unsigned int index = y_int*m_width + x_int;
    *value = m_data[index];
    *color = m_color_conversion_table[ m_colors[index] ];
};
