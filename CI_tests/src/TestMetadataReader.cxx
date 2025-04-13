#include "../headers/TestMetadataReader.h"

#include "../../headers/MetadataReader.h"

#include <iostream>

using namespace AstroPhotoStacker;

TestResult AstroPhotoStacker::test_metadata_reading(const std::string &input_file,
                                                    float expected_aperture,
                                                    float expected_exposure_time,
                                                    int expected_iso,
                                                    float expected_focal_length,
                                                    int expected_unix_time)    {

    const Metadata metadata = read_metadata(InputFrame(input_file));
    std::string error_message = "";

    if (metadata.aperture != expected_aperture) {
        error_message += "Aperture mismatch: expected " + std::to_string(expected_aperture) + ", got " + std::to_string(metadata.aperture) + "\n";
    }
    if (metadata.exposure_time != expected_exposure_time) {
        error_message += "Exposure time mismatch: expected " + std::to_string(expected_exposure_time) + ", got " + std::to_string(metadata.exposure_time) + "\n";
    }
    if (metadata.iso != expected_iso) {
        error_message += "ISO mismatch: expected " + std::to_string(expected_iso) + ", got " + std::to_string(metadata.iso) + "\n";
    }
    if (metadata.focal_length != expected_focal_length) {
        error_message += "Focal length mismatch: expected " + std::to_string(expected_focal_length) + ", got " + std::to_string(metadata.focal_length) + "\n";
    }
    if (metadata.timestamp != expected_unix_time && expected_unix_time >= 0) {
        error_message += "Date time mismatch: expected " + std::to_string(expected_unix_time) + ", got " + std::to_string(metadata.timestamp) + "\n";
    }
    if (error_message.empty()) {
        return TestResult(true, "Metadata reading test passed.");
    } else {
        return TestResult(false, error_message);
    }
};

