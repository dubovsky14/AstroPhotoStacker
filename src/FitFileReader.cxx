#include "../headers/FitFileReader.h"
#include "../headers/Common.h"

#include <iostream>
#include <algorithm>
#include <numeric>

using namespace AstroPhotoStacker;
using namespace std;

FitFileReader::FitFileReader(const std::string &input_file) :
    FitFileMetadataReader(input_file) {
    read_data(*m_input_stream);
};

void FitFileReader::read_data(std::ifstream &file) {
    const unsigned  int n_elements = m_width*m_height;
    m_data = std::vector<unsigned short int>(n_elements);

    // skip whitespaces
    char x;
    while (file.peek() == ' ') {
        file.read(&x, 1);
    }

    if (m_bit_depth == 16) {
        read_data_16bit(file);
    }
    else if (m_bit_depth == 8) {
        read_data_8bit(file);
    }
    else {
        throw std::invalid_argument("Unsupported bit depth");
    }
};

void FitFileReader::read_data_16bit(std::ifstream &file)   {

    file.read(reinterpret_cast<char*>(m_data.data()), m_data.size()*2);

    // data are not stored as usual 16 bit unsigned integers - first byte is the least significant one, second byte is the most significant one
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned short int pixel_value) -> unsigned short int {
        return pixel_value >> 8 | pixel_value << 8;
    });

    // subtract zero point
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [this](unsigned short int x) -> unsigned short {return x + m_zero_point;});

    // we need this to be able to convert it to 16 bit signed integers (-1 is "no value")
    // do not worry, if the resolution is 14 bits for example, the 2 least significant bits will be 0 (and not the two most significant ones)
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [this](unsigned short int x) -> unsigned short {return x/2;});
};

void FitFileReader::read_data_8bit(std::ifstream &file) {
    vector<char> signed_char_buffer(m_width*m_height);
    file.read(reinterpret_cast<char*>(signed_char_buffer.data()), signed_char_buffer.size());

    std::transform(signed_char_buffer.begin(), signed_char_buffer.end(), m_data.begin(), [this](char pixel_value) -> unsigned short int {
        return static_cast<short unsigned int>(static_cast<short int>(pixel_value) + m_zero_point);
    });
};
