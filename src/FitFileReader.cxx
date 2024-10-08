#include "../headers/FitFileReader.h"
#include "../headers/Common.h"

#include <iostream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

FitFileReader::FitFileReader(const std::string &input_file)    {
    std::ifstream file(input_file, std::ios::binary | std::ios::in);
    if (!file.is_open())    {
        throw std::runtime_error("Could not open file " + input_file);
    }

    read_header(file);

    read_data(file);
};

std::vector<char> FitFileReader::get_colors() const    {
    vector<char> result(m_width*m_height);
    for (int i_y = 0; i_y < m_height; i_y++) {
        for (int i_x = 0; i_x < m_width; i_x++) {
            const unsigned int i_pixel = i_y*m_width + i_x;
            const unsigned int i_color = get_color(i_x, i_y);
            result[i_pixel] = i_color;
        }
    }
    return result;
};

void FitFileReader::read_header(std::ifstream &file)   {
    const string header = get_header_string(file);
    parse_header(header);

    try {
        fill_metadata();
    }
    catch (const std::invalid_argument &e) {
        throw std::runtime_error(e.what());
    }
};

void FitFileReader::parse_header(const std::string &header)    {
    int first_non_whitespace = -1;
    int last_non_whitespace = -1;
    bool reading_value = false;
    string key, value;
    bool previous_was_space = false;

    for (unsigned int i = 0; i < header.size(); i++) {
       // reading key
        if (header[i] == ' ') {
            previous_was_space = true;
            continue;
        }
        if (header[i] != ' ' && previous_was_space && !reading_value && header[i] != '=') {
            first_non_whitespace = i;
            last_non_whitespace = i;
            previous_was_space = false;
        }
        else if (header[i] != '=' && !reading_value){
            if (first_non_whitespace == -1) {
                first_non_whitespace = i;
            }
            last_non_whitespace = i;
        }
        // = sign, start reading value
        else if (header[i] == '=' && !reading_value) {
            key = header.substr(first_non_whitespace, last_non_whitespace - first_non_whitespace + 1);
            first_non_whitespace = -1;
            last_non_whitespace = -1;
            reading_value = true;
        }
        else if (reading_value) {
            if (header[i] != '/')   {
                value += header[i];
            }
            else {
                strip_string(&value, "\'\"");
                m_metadata[key] = value;
                key = "";
                value = "";
                reading_value = false;
            }
        }
    }
};

std::string FitFileReader::get_header_string(std::ifstream &file)    {
    string header;
    char x;
    while (!ends_with(header, " END "))    {
        file.read(&x, 1);
        header += x;
    }
    return header;
};

void FitFileReader::fill_metadata()    {
    m_width  = std::stoi(get_with_default<string,string>(m_metadata, "NAXIS1", "0"));
    m_height = std::stoi(get_with_default<string,string>(m_metadata, "NAXIS2", "0"));
    m_bit_depth = std::stoi(get_with_default<string,string>(m_metadata, "BITPIX", "0"));
    m_exposure_time = std::stof(get_with_default<string,string>(m_metadata, "EXPTIME", "0"));

    const string bayer_matrix = get_with_default<string,string>(m_metadata, "BAYERPAT", "");
    process_bayer_matrix(bayer_matrix);

    // get zero point
    m_zero_point = std::stoi(get_with_default<string,string>(m_metadata, "BZERO", "0"));

    // just for output metadata struct:
    m_metadata_struct.aperture = std::stof(get_with_default<string,string>(m_metadata, "APERTURE", "0"));
    m_metadata_struct.exposure_time = m_exposure_time;
    m_metadata_struct.iso = std::stoi(get_with_default<string,string>(m_metadata, "ISO", "0"));
    if (m_metadata_struct.iso == 0) {
        m_metadata_struct.iso = std::stoi(get_with_default<string,string>(m_metadata, "GAIN", "0"));
    }

    m_metadata_struct.focal_length = std::stof(get_with_default<string,string>(m_metadata, "FOCALLEN", "0"));
    m_metadata_struct.date_time = get_with_default<string,string>(m_metadata, "DATE-OBS", "");
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
        file.read((char *)&m_data[(m_height - i_line - 1)*m_width], m_width*2);
    }

    // subtract zero point
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [this](unsigned short int x) {return x - m_zero_point;});

    if (m_is_rgb) {
        apply_green_correction();
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

void FitFileReader::process_bayer_matrix(const std::string &bayer_matrix)  {
    string bayer_matrix_upper = to_upper_copy(bayer_matrix);
    strip_string(&bayer_matrix_upper, " \n\t\r\'\"");

    if (bayer_matrix.empty()) {
        m_is_rgb = false;
        return;
    }

    if (bayer_matrix_upper.length() != 4) {
        throw std::invalid_argument("Bayer matrix must have 4 characters. Matrix: '"s + bayer_matrix + "'");
    }
    for (int i = 0; i < 4; i++) {
        const char c = bayer_matrix_upper[i];
        if (c == 'R') {
            m_bayer_matrix[i] = 0;
        }
        else if (c == 'G') {
            m_bayer_matrix[i] = 1;
        }
        else if (c == 'B') {
            m_bayer_matrix[i] = 2;
        }
        else {
            throw std::invalid_argument("Invalid Bayer matrix character");
        }
    }
    m_is_rgb = true;
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