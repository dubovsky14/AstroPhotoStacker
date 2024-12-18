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

    // why the fuck do some FIT files come with 16 bit precision? This is absurd, it's just a noise causing problems ...
    shrink_to_15_bits();
};

void FitFileReader::shrink_to_15_bits()    {
    m_bit_depth = 15;
    for (unsigned int i = 0; i < m_data.size(); i++) {
        m_data[i] = m_data[i] >> 1;
    }
};