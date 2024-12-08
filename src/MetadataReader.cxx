#include "../headers/MetadataReader.h"

#include <exiv2/exiv2.hpp>
#include <string>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;


Metadata AstroPhotoStacker::read_metadata_video(const std::string &input_file) {
    Metadata metadata;
    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(input_file);
        if (image.get() == 0) {
            throw std::runtime_error("Failed to open the file " + input_file);
        }
        image->readMetadata();

        Exiv2::ExifData &exifData = image->exifData();
        if (exifData.empty()) {
            std::string error(input_file);
            error += ": No EXIF data found in the file";
            throw Exiv2::Error(Exiv2::kerErrorMessage, error);
        }

        // Date and Time
        const auto dateTime = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
        if (dateTime != exifData.end()) {
            metadata.date_time = dateTime->toString();
            metadata.timestamp = get_unix_timestamp(metadata.date_time);
            cout << "Timestamp: "s + to_string(metadata.timestamp) + "\n"s;
        } else {
            std::cerr << "DateTimeOriginal not found in the metadata." << std::endl;
        }

    } catch (Exiv2::AnyError& e) {
        std::cerr << "Error reading file " << input_file << ": " << e.what() << std::endl;

        // get at least the timestamp
        metadata = get_file_creation_timestamp(input_file, metadata);
    }
    return metadata;
}


Metadata AstroPhotoStacker::read_metadata_rgb_image(const std::string &input_file)   {
    Metadata metadata;
    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(input_file);
        if (image.get() == 0)   {
            throw std::runtime_error("Failed to open the file"s + input_file);

        };
        image->readMetadata();

        Exiv2::ExifData &exifData = image->exifData();
        if (exifData.empty()) {
            std::string error(input_file);
            error += ": No EXIF data found in the file";
            throw Exiv2::Error(Exiv2::kerErrorMessage, error);
        }

        // Aperture
        const auto aperture = exifData.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
        if (aperture != exifData.end()) {
            metadata.aperture = aperture->toFloat();
        }

        // Exposure Time
        const auto exposureTime = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime"));
        if (exposureTime != exifData.end()) {
            metadata.exposure_time = exposureTime->toFloat();
        }

        // ISO Speed
        const auto isoSpeed = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"));
        if (isoSpeed != exifData.end()) {
            metadata.iso = isoSpeed->toLong();
        }

        // Focal Length
        const auto focalLength = exifData.findKey(Exiv2::ExifKey("Exif.Photo.FocalLength"));
        if (focalLength != exifData.end()) {
            metadata.focal_length= focalLength->toFloat();
        }

        // Date and Time
        const auto dateTime = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
        if (dateTime != exifData.end()) {
            metadata.date_time = dateTime->toString();
            metadata.timestamp = get_unix_timestamp(metadata.date_time);
        }

        // Max Value
        const auto maxValue = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureBiasValue"));
        if (maxValue != exifData.end()) {
            metadata.max_value = maxValue->toLong();
        }

    } catch (Exiv2::AnyError& e) {
        std::cerr << "Error reading file " << input_file << ": " << e.what() << std::endl;
    }
    return metadata;
};

Metadata AstroPhotoStacker::read_metadata(const InputFrame &input_frame)    {
    const std::string input_file = input_frame.get_file_address();
    if (input_frame.is_video_frame()) {
        return read_metadata_video(input_file);
    }
    const bool raw_file = is_raw_file(input_file);
    if (raw_file)    {
        return read_metadata_from_raw_file(input_file);
    }
    else    {
        return read_metadata_rgb_image(input_file);
    }
};

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