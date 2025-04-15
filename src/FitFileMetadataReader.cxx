#include "../headers/FitFileMetadataReader.h"
#include "../headers/Common.h"

#include <iostream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

FitFileMetadataReader::FitFileMetadataReader(const std::string &input_file)    {
    m_input_stream = std::make_unique<std::ifstream>(input_file, std::ios::binary | std::ios::in);
    if (!m_input_stream->is_open())    {
        throw std::runtime_error("Could not open file " + input_file);
    }

    read_header(*m_input_stream);
};

int FitFileMetadataReader::get_unix_timestamp(const std::string &time_string)   {
    struct tm tm;
    string time_string_adjusted = time_string;
    for (unsigned int i = 0; i < time_string_adjusted.size(); i++) {
        if (time_string_adjusted[i] == 'T') {
            time_string_adjusted[i] = ' ';
        }
        if (time_string_adjusted[i] == '.') {
            time_string_adjusted = time_string_adjusted.substr(0, i);
            break;
        }
    }

    strptime(time_string.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    return mktime(&tm);
};

std::vector<char> FitFileMetadataReader::get_colors() const    {
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

void FitFileMetadataReader::read_header(std::ifstream &file)   {
    const string header = get_header_string(file);
    parse_header(header);

    try {
        fill_metadata();
    }
    catch (const std::invalid_argument &e) {
        throw std::runtime_error(e.what());
    }
};

void FitFileMetadataReader::parse_header(const std::string &header)    {
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
        // starting to read key
        if (header[i] != ' ' && previous_was_space && !reading_value && header[i] != '=') {
            first_non_whitespace = i;
            last_non_whitespace = i;
            previous_was_space = false;
        }
        // reading key
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

std::string FitFileMetadataReader::get_header_string(std::ifstream &file)    {
    string header;
    char x;
    while (!ends_with(header, " END "))    {
        file.read(&x, 1);
        header += x;
    }
    return header;
};

void FitFileMetadataReader::fill_metadata()    {
    m_width  = std::stoi(get_with_default<string,string>(m_metadata, "NAXIS1", "0"));
    m_height = std::stoi(get_with_default<string,string>(m_metadata, "NAXIS2", "0"));
    m_bit_depth = std::stoi(get_with_default<string,string>(m_metadata, "BITPIX", "16"));
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
    m_metadata_struct.timestamp = FitFileMetadataReader::get_unix_timestamp(m_metadata_struct.date_time);
    m_metadata_struct.monochrome = bayer_matrix == "";
};

void FitFileMetadataReader::process_bayer_matrix(const std::string &bayer_matrix)  {
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
