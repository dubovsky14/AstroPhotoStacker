#include "../headers/MetadataCommon.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;


int AstroPhotoStacker::get_unix_timestamp(const std::string &time_string)  {
    struct tm tm;
    strptime(time_string.c_str(), "%Y:%m:%d %H:%M:%S", &tm);
    return mktime(&tm);
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