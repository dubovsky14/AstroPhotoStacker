#include "../headers/RawFileReaderFit.h"

#include "../headers/Common.h"
#include "../headers/MetadataCommon.h"

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


bool AstroPhotoStacker::is_fit_file(const std::string &file_address) {
    const string file_address_upper = to_upper_copy(file_address);
    return ends_with(file_address_upper, ".FIT") || ends_with(file_address_upper, ".FITS");
}

RawFileReaderFit::RawFileReaderFit(const InputFrame &input_frame) : RawFileReaderBase(input_frame) {};

std::vector<PixelType> RawFileReaderFit::read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) {
    ifstream input_stream(m_input_frame.get_file_address(), ios::binary | ios::in);
    if (!input_stream.is_open())    {
        throw std::runtime_error("Could not open file " + m_input_frame.get_file_address());
    }
    read_header(&input_stream);
    read_data(&input_stream);

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }
    if (bayer_pattern != nullptr) {
        *bayer_pattern = m_bayer_matrix;
    }
    return m_data;
};

void RawFileReaderFit::get_photo_resolution(int *width, int *height) {
    ifstream input_stream(m_input_frame.get_file_address(), ios::binary | ios::in);
    if (!input_stream.is_open())    {
        throw std::runtime_error("Could not open file " + m_input_frame.get_file_address());
    }
    read_header(&input_stream);

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }
};

Metadata RawFileReaderFit::read_metadata_without_cache() {
    ifstream input_stream(m_input_frame.get_file_address(), ios::binary | ios::in);
    if (!input_stream.is_open())    {
        throw std::runtime_error("Could not open file " + m_input_frame.get_file_address());
    }
    read_header(&input_stream);

    return m_metadata;
};

void RawFileReaderFit::read_data(std::ifstream *file) {
    const unsigned  int n_elements = m_width*m_height;
    m_data = std::vector<PixelType>(n_elements);

    // skip whitespaces
    char x;
    while (file->peek() == ' ') {
        file->read(&x, 1);
    }

    if (m_bit_depth == 16) {
        read_data_16bit(file);
    }
    else if (m_bit_depth == 8) {
        read_data_8bit(file);
    }
    else {
        throw std::invalid_argument("Unsupported bit depth in FIT file: " + m_input_frame.get_file_address());
    }
};

void RawFileReaderFit::read_data_16bit(std::ifstream *file)   {
    vector<unsigned short int> data_unsigned(m_width*m_height);

    file->read(reinterpret_cast<char*>(data_unsigned.data()), data_unsigned.size()*2);

    // data are not stored as usual 16 bit unsigned integers - first byte is the least significant one, second byte is the most significant one
    std::transform(data_unsigned.begin(), data_unsigned.end(), data_unsigned.begin(), [](unsigned short int pixel_value) -> unsigned short int {
        return pixel_value >> 8 | pixel_value << 8;
    });

    // subtract zero point
    std::transform(data_unsigned.begin(), data_unsigned.end(), data_unsigned.begin(), [this](unsigned short int x) -> unsigned short {return x + m_zero_point;});

    // we need this to be able to convert it to 16 bit signed integers (-1 is "no value")
    // do not worry, if the resolution is 14 bits for example, the 2 least significant bits will be 0 (and not the two most significant ones)
    std::transform(data_unsigned.begin(), data_unsigned.end(), m_data.begin(), [](unsigned short int x) -> short {return x/2;});
};

void RawFileReaderFit::read_data_8bit(std::ifstream *file) {
    vector<char> signed_char_buffer(m_width*m_height);
    file->read(reinterpret_cast<char*>(signed_char_buffer.data()), signed_char_buffer.size());

    std::transform(signed_char_buffer.begin(), signed_char_buffer.end(), m_data.begin(), [this](char pixel_value) -> unsigned short int {
        return static_cast<short unsigned int>(static_cast<PixelType>(pixel_value) + m_zero_point);
    });

    scale_8bit_image_to_16bit(&m_data);
};


void RawFileReaderFit::read_header(std::ifstream *file)   {
    const string header = get_header_string(file);
    parse_header(header);

    try {
        fill_metadata();
    }
    catch (const std::invalid_argument &e) {
        throw std::runtime_error(e.what());
    }
};

void RawFileReaderFit::parse_header(const std::string &header)    {
    int first_non_whitespace = -1;
    int last_non_whitespace = -1;
    bool reading_value = false;
    string key, value;
    bool previous_was_space = false;

    for (unsigned int i = 0; i < header.size(); i++) {
       // reading key
        if (header[i] == ' ') {
            previous_was_space = true;
            if (!reading_value) {
                continue;
            }
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
                strip_string(&value, "\'\" ");
                m_metadata_map[key] = value;
                key = "";
                value = "";
                reading_value = false;
            }
        }
    }
};

std::string RawFileReaderFit::get_header_string(std::ifstream *file)    {
    string header;
    char x;
    while (!ends_with(header, " END ") && file->read(&x, 1)) {
        header += x;
    }
    return header;
};

void RawFileReaderFit::fill_metadata()    {
    m_width  = std::stoi(get_with_default<string,string>(m_metadata_map, "NAXIS1", "0"));
    m_height = std::stoi(get_with_default<string,string>(m_metadata_map, "NAXIS2", "0"));
    m_bit_depth = std::stoi(get_with_default<string,string>(m_metadata_map, "BITPIX", "16"));
    m_metadata.exposure_time = std::stof(get_with_default<string,string>(m_metadata_map, "EXPTIME", "0"));

    const string bayer_matrix = get_with_default<string,string>(m_metadata_map, "BAYERPAT", "");
    process_bayer_matrix(bayer_matrix);

    // get zero point
    m_zero_point = std::stoi(get_with_default<string,string>(m_metadata_map, "BZERO", "0"));

    // just for output metadata struct:
    m_metadata.aperture = std::stof(get_with_default<string,string>(m_metadata_map, "APERTURE", "0"));
    m_metadata.iso = std::stoi(get_with_default<string,string>(m_metadata_map, "ISO", "0"));
    if (m_metadata.iso == 0) {
        m_metadata.iso = std::stoi(get_with_default<string,string>(m_metadata_map, "GAIN", "0"));
    }

    m_metadata.focal_length = std::stof(get_with_default<string,string>(m_metadata_map, "FOCALLEN", "0"));
    const std::string date_time_string = get_with_default<string,string>(m_metadata_map, "DATE-OBS", "");
    m_metadata.timestamp = RawFileReaderFit::get_unix_timestamp(date_time_string);
    m_metadata.monochrome = bayer_matrix == "";
    m_metadata.bayer_matrix = convert_bayer_int_array_to_string(m_bayer_matrix);
    m_metadata.is_raw = true;
    m_metadata.camera_model = get_with_default<string,string>(m_metadata_map, "INSTRUME", "");
    m_metadata.temperature = std::stof(get_with_default<string,string>(m_metadata_map, "CCD-TEMP", "-300"));
};

void RawFileReaderFit::process_bayer_matrix(const std::string &bayer_matrix)  {
    string bayer_matrix_upper = to_upper_copy(bayer_matrix);
    strip_string(&bayer_matrix_upper, " \n\t\r\'\"");

    if (bayer_matrix_upper.length() != 4) {
        throw std::invalid_argument("Bayer matrix must have 4 characters. Matrix: '"s + bayer_matrix + "'. File: " + m_input_frame.get_file_address());
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
            throw std::invalid_argument("Invalid Bayer matrix character in FIT file: " + m_input_frame.get_file_address());
        }
    }
};

int RawFileReaderFit::get_unix_timestamp(const std::string &time_string)   {
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

    strptime(time_string_adjusted.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    return timegm(&tm);
};
