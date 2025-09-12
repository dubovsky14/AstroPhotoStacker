#include "../headers/FitFileSaver.h"

#include "../headers/Metadata.h"
#include "../headers/MetadataCommon.h" // get_string_timestamp_from_unix_time

using namespace AstroPhotoStacker;
using namespace std;

FitFileSaver::FitFileSaver(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Width and height must be positive integers.");
    }
    m_width = width;
    m_height = height;
};

void FitFileSaver::set_bits_per_pixel(int bits_per_pixel)     {
    if (bits_per_pixel == 8)    {
        m_b_zero = 128;
    }
    else if (bits_per_pixel == 16) {
        m_b_zero = 32768;
    }
    else {
        throw std::invalid_argument("Unsupported bits per pixel value.");
    }
    m_bits_per_pixel = bits_per_pixel;
};

std::string FitFileSaver::get_header_string() const {
    std::string header_string;
    header_string += get_header_line("SIMPLE", "T", "Standard FITS format");
    header_string += get_header_line("BITPIX", std::to_string(m_bits_per_pixel), "Bits per pixel");
    header_string += get_header_line("NAXIS", "2", "Number of axes");
    header_string += get_header_line("NAXIS1", std::to_string(m_width), "Width of image");
    header_string += get_header_line("NAXIS2", std::to_string(m_height), "Height of image");
    header_string += get_header_line("BZERO", std::to_string(m_b_zero), "Zero level");
    header_string += get_header_line("BSCALE", "1", "default scaling factor");

    const std::string bayer_matrix = m_metadata.has_value() ? m_metadata->bayer_matrix : "";
    if (!bayer_matrix.empty()) {
        header_string += get_header_line("BAYERPAT", bayer_matrix, "Bayer pattern");
    }
    else    {
        header_string += get_header_line("BAYERPAT", "\'RGGB\'", "Bayer pattern");
    }

    const std::string camera_model = m_metadata.has_value() ? m_metadata->camera_model : "";
    if (!camera_model.empty()) {
        header_string += get_header_line("INSTRUME", camera_model, "Camera model");
    }


    header_string += get_metadata_header_string();

    add_final_header_padding(&header_string);

    return header_string;
};

std::string FitFileSaver::get_padded_value(const std::string &value)        const  {
    return get_padded_text(value, 22, true);
};

std::string FitFileSaver::get_padded_comment(const std::string &comment)    const  {
    return get_padded_text(comment, 48, false);
};

std::string FitFileSaver::get_padded_key(const std::string &key)  const   {
    if (key.length() > 8) {
        throw std::invalid_argument("Key is too long to be padded.");
    }
    std::string padded_key = key;
    padded_key += std::string(8 - key.length(), ' ');
    return padded_key;
};

std::string FitFileSaver::get_padded_text(const std::string &text, int padded_length, bool padding_on_left)  const  {
    if (int(text.length()) > padded_length-2) {
        throw std::invalid_argument("Text is too long to be padded.");
    }
    const int padding_length = padded_length;
    string padded_text = " " + text + " ";
    if (padding_on_left) {
        padded_text = std::string(padding_length - padded_text.length(), ' ') + padded_text;
    } else {
        padded_text = padded_text + std::string(padding_length - padded_text.length(), ' ');
    }
    return padded_text;
};

std::string FitFileSaver::get_header_line(const std::string &key, const std::string &value, const std::string &comment)  const  {
    std::string header_line = get_padded_key(key) + "=" + get_padded_value(value) + "/" + get_padded_comment(comment);
    return header_line;
};

std::string FitFileSaver::get_metadata_header_string() const  {
    if (!m_metadata.has_value()) return "";

    std::string metadata_string;

    const float exposure_time = m_metadata->exposure_time;
    const std::string exposure_time_str = std::to_string(exposure_time);
    metadata_string += get_header_line("EXPOSURE", exposure_time_str,               "Exposure time in seconds");
    metadata_string += get_header_line("EXPTIME",  exposure_time_str,               "Exposure time in seconds");
    metadata_string += get_header_line("GAIN",     std::to_string(m_metadata->iso), "Gain (ISO) value");

    const std::string date_time_str = get_string_timestamp_from_unix_time(m_metadata->timestamp, "%Y-%m-%dT%H:%M:%S");
    metadata_string += get_header_line("DATE-OBS", date_time_str,           "Image exposure start time");

    if (m_metadata->focal_length > 0) {
        metadata_string += get_header_line("FOCALLEN", std::to_string(m_metadata->focal_length), "Focal length in mm");
    }

    if (m_metadata->aperture > 0) {
        metadata_string += get_header_line("APERTURE", std::to_string(m_metadata->aperture), "Aperture in mm");
    }

    return metadata_string;
};

void FitFileSaver::add_final_header_padding(std::string *header_string)  const  {
    header_string->append("END");
    const int padding_length = 2880 - header_string->length() % 2880;
    if (padding_length > 0) {
        header_string->append(padding_length, ' ');
    }
};