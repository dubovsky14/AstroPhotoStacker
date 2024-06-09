#include "../headers/FitFileReader.h"
#include "../headers/Common.h"

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

void FitFileReader::read_header(std::ifstream &file)   {
    const string header = get_header_string(file);
    parse_header(header);
    fill_metadata();
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
};

void FitFileReader::read_data(std::ifstream &file) {
    const int n_elements = m_width*m_height;
    m_data = std::make_unique<unsigned short int[]>(n_elements);

    // skip whitespaces
    char x;
    while (file.peek() == ' ') {
        file.read(&x, 1);
    }
    int current_pos = file.tellg();

    // shift stream one byte back
    file.seekg(current_pos - 1);

    file.read((char *)m_data.get(), n_elements*sizeof(unsigned short int));
};