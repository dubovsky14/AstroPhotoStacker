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


    file.read(reinterpret_cast<char*>(m_data.data()), n_elements*2);

    // data are not stored as usual 16 bit unsigned integers - first byte is the least significant one, second byte is the most significant one
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned short int pixel_value) -> unsigned short int {
        return pixel_value >> 8 | pixel_value << 8;
    });

    // subtract zero point
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [this](unsigned short int x) {return x - m_zero_point;});

    // we need this to be able to convert it to 16 bit signed integers (-1 is "no value")
    // do not worry, if the resolution is 14 bits for example, the 2 least significant bits will be 0 (and not the two most significant ones)
    shrink_to_15_bits();
};

void FitFileReader::shrink_to_15_bits()    {
    m_bit_depth = 15;
    for (unsigned int i = 0; i < m_data.size(); i++) {
        m_data[i] = m_data[i]/2;
    }
};

void FitFileReader::apply_color_balance()   {
    vector<vector<unsigned short int>> histogram(3, vector<unsigned short int>(65535,0));

    const unsigned int n_pixels = m_width*m_height;
    for (unsigned int i = 0; i < n_pixels; i++) {
        histogram[get_color(i/m_width, i%m_width)][m_data[i]]++;
    }

    int pixels_per_channel[3] = {0, 0, 0};
    for (unsigned int i_color = 0; i_color < 3; i_color++) {
        pixels_per_channel[i_color] = std::accumulate(histogram[i_color].begin(), histogram[i_color].end(), 0);
    }

    double averages[3] = {0, 0, 0};
    for (unsigned int i_color = 0; i_color < 3; i_color++) {
        for (unsigned int i = 0; i < histogram[i_color].size(); i++) {
            averages[i_color] += i*histogram[i_color][i];
        }
        averages[i_color] /= pixels_per_channel[i_color];
    }
    const float sf_red = averages[1]/averages[0];
    const float sf_blue = averages[1]/averages[2];

    for (int i_x = 0; i_x < m_width; i_x++) {
        for (int i_y = 0; i_y < m_height; i_y++) {
            const unsigned int i_pixel = i_y*m_width + i_x;
            const unsigned int i_color = get_color(i_x, i_y);
            if (i_color == 0) {
                m_data[i_pixel] = std::min<double>(65535, m_data[i_pixel]*sf_red);
            }
            else if (i_color == 2) {
                m_data[i_pixel] = std::min<double>(65535, m_data[i_pixel]*sf_blue);
            }
        }
    }
};