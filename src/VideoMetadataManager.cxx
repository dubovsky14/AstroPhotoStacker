#include "../headers/VideoMetadataManager.h"
#include "../headers/CustomSharedMutex.h"
#include "../headers/MetadataCommon.h"

#include "../headers/VideoReader.h"


#include <exiv2/exiv2.hpp>

using namespace AstroPhotoStacker;
using namespace std;

Metadata VideoMetadataManager::get_metadata(const InputFrame &input_frame) {
    const string input_file = input_frame.get_file_address();
    const int frame_number = input_frame.get_frame_number();

    auto get_metadata_lambda = [this, &input_file]() {
        return read_metadata_video(input_file);
    };

    Metadata metadata = m_metadata_cache.get(input_file, get_metadata_lambda);
    if (frame_number < 0) {
        return metadata; // still image, no need to adjust timestamp
    }

    const float fps = metadata.video_fps;
    metadata.timestamp += int(frame_number / fps); // add time in video to to the original timestamp
    return metadata;
}

Metadata VideoMetadataManager::read_metadata_video_non_raw(const std::string &input_file)   const {
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

Metadata VideoMetadataManager::read_metadata_video(const string &input_file) const {
    const bool raw_file = is_raw_file(input_file);
    if (raw_file) {
        return read_metadata_from_raw_file(input_file);
    }
    return read_metadata_video_non_raw(input_file);
};

