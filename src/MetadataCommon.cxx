#include "../headers/MetadataCommon.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;


int AstroPhotoStacker::get_unix_timestamp(const std::string &time_string, const std::string timestamp_format)  {
    struct tm tm;
    strptime(time_string.c_str(), timestamp_format.c_str(), &tm);
    return mktime(&tm);
};

std::string AstroPhotoStacker::get_string_timestamp_from_unix_time(int unix_time, const std::string timestamp_format)    {
    std::time_t time = static_cast<std::time_t>(unix_time);
    std::tm *tm = std::localtime(&time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), timestamp_format.c_str(), tm);
    return std::string(buffer);
};

Metadata AstroPhotoStacker::get_file_creation_timestamp(const std::string &file_address, const Metadata &other_metadata)  {
    Metadata metadata = other_metadata;

    auto ftime = filesystem::last_write_time(file_address);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    // convert to string
    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%Y:%m:%d %H:%M:%S");
    metadata.date_time = ss.str();
    metadata.timestamp = get_unix_timestamp(metadata.date_time);

    return metadata;
};

std::string AstroPhotoStacker::convert_bayer_int_array_to_string(const std::array<char, 4> &bayer_array) {
    std::string bayer_string;
    for (const char &c : bayer_array) {
        char color_letter = ' ';
        switch (c) {
            case 0:
                color_letter = 'R';
                break;
            case 1:
                color_letter = 'G';
                break;
            case 2:
                color_letter = 'B';
                break;
            case 3:
                color_letter = 'G';
                break;
            default:
                throw std::invalid_argument("Invalid Bayer matrix value");

        }
        bayer_string += color_letter;
    }
    return bayer_string;
}