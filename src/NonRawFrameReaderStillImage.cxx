#include "../headers/NonRawFrameReaderStillImage.h"
#include "../headers/MetadataCommon.h"

#include <exiv2/exiv2.hpp>
#include <opencv2/opencv.hpp>

using namespace AstroPhotoStacker;
using namespace std;

NonRawFrameReaderStillImage::NonRawFrameReaderStillImage(const InputFrame &input_frame) : NonRawFrameReaderBase(input_frame) {
};

vector<vector<PixelType>> NonRawFrameReaderStillImage::get_pixels_data(int *width, int *height) {
    const string input_file = m_input_frame.get_file_address();
    cv::Mat image = cv::imread(input_file, cv::IMREAD_COLOR | cv::IMREAD_ANYDEPTH);
    if (image.empty()) {
        throw std::runtime_error("Unable to open image file: " + input_file);
    }

    vector<vector<PixelType>> result = opencv_rgb_image_to_vector_vector_short(image, &m_width, &m_height);

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }

    return result;
};

std::vector<PixelType> NonRawFrameReaderStillImage::get_pixels_data_monochrome(int *width, int *height) {
    const string input_file = m_input_frame.get_file_address();
    cv::Mat image = cv::imread(input_file, cv::IMREAD_ANYDEPTH);
    if (image.empty()) {
        throw std::runtime_error("Unable to open image file: " + input_file);
    }

    vector<PixelType> result = opencv_grayscale_image_to_vector_short(image, &m_width, &m_height);

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }

    return result;
};

void NonRawFrameReaderStillImage::get_photo_resolution(int *width, int *height) {
    if (m_width < 0 || m_height < 0) {
        const string input_file = m_input_frame.get_file_address();
        cv::Mat image = cv::imread(input_file, cv::IMREAD_COLOR | cv::IMREAD_ANYDEPTH);
        m_width = image.cols;
        m_height = image.rows;
    }
    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }
};

Metadata NonRawFrameReaderStillImage::read_metadata_without_cache() {
    const string input_file = m_input_frame.get_file_address();
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


        // print list of keys
        for (const auto& key : exifData) {
            std::cout << key.key() << " " << key.value() << std::endl;
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

        // camera model
        const auto cameraModel = exifData.findKey(Exiv2::ExifKey("Exif.Image.Model"));
        if (cameraModel != exifData.end()) {
            metadata.camera_model = cameraModel->toString();
        }

    } catch (Exiv2::AnyError& e) {
        std::cerr << "Error reading file " << input_file << ": " << e.what() << std::endl;
    }
    return metadata;
};