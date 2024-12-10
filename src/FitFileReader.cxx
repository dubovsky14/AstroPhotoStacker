#include "../headers/FitFileReader.h"
#include "../headers/Common.h"

#include <iostream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

FitFileReader::FitFileReader(const std::string &input_file) :
    FitFileMetadataReader(input_file) {
    read_data(*m_input_stream);
};

void FitFileReader::read_data(std::ifstream &file) {
    const int n_elements = m_width*m_height;
    m_data = std::vector<unsigned short int>(n_elements);

    // skip whitespaces
    char x;
    while (file.peek() == ' ') {
        file.read(&x, 1);
    }
    int current_pos = file.tellg();

    // shift stream one byte back
    file.seekg(current_pos - 1);

    // lines are indexed from bottom to top ...
    for (int i_line = 0; i_line < m_height; i_line++) {
        file.read((char *)&m_data[(i_line)*m_width], m_width*2);
    }

    // subtract zero point
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [this](unsigned short int x) {return x - m_zero_point;});

    if (m_is_rgb) {
        //apply_green_correction();
    }

    // why the fuck do some FIT files come with 16 bit precision? This is absurd, it's just a noise causing problems ...
    shrink_to_15_bits();
};

void FitFileReader::shrink_to_15_bits()    {
    if (m_bit_depth == 16) {
        for (unsigned int i = 0; i < m_data.size(); i++) {
            m_data[i] = m_data[i] >> 1;
        }
    }
    m_bit_depth = 15;
};

void FitFileReader::apply_green_correction()   {
    if (!m_is_rgb) {
        return;
    }
    const int max_value = (2 << (m_bit_depth - 1)) - 1;

    for (int i_y = 0; i_y < m_height; i_y++) {
        for (int i_x = 0; i_x < m_width; i_x++) {
            const unsigned int i_color = get_color(i_x, i_y);
            if (i_color == 1) {
                //m_data[i_y*m_width + i_x] = std::min<int>(2*m_data[i_y*m_width + i_x], max_value/2);
            }
            else  {
                m_data[i_y*m_width + i_x] = std::min<int>(0.5*m_data[i_y*m_width + i_x], max_value/2);
            }
        }
    }

};