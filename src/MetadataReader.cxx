#include "../headers/MetadataReader.h"

#include <exiv2/exiv2.hpp>
#include <string>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

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

    } catch (Exiv2::AnyError& e) {
        std::cerr << "Error reading file " << input_file << ": " << e.what() << std::endl;
    }
    return metadata;
};

Metadata AstroPhotoStacker::read_metadata(const std::string &input_file)    {
    const bool raw_file = is_raw_file(input_file);
    if (raw_file)    {
        return read_metadata_from_raw_file(input_file);
    }
    else    {
        return read_metadata_rgb_image(input_file);
    }
};