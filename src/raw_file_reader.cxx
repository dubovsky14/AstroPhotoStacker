#include "../headers/raw_file_reader.h"

#include "../headers/FitFileMetadataReader.h"
#include "../headers/VideoReaderSer.h"
#include "../headers/MetadataCommon.h"
#include "../headers/Common.h"

#include <libraw/libraw.h>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace AstroPhotoStacker;

bool AstroPhotoStacker::is_fit_file(const std::string &file_address)   {
    return ends_with(to_upper_copy(file_address), ".FIT");
};

Metadata AstroPhotoStacker::read_metadata_from_raw_file(const std::string &raw_file_address) {
    if (is_fit_file(raw_file_address)) {
        FitFileMetadataReader fit_file_metadata_reader(raw_file_address);
        return fit_file_metadata_reader.get_metadata();
    }
    if (AstroPhotoStacker::is_ser_file(raw_file_address)) {
        return read_ser_video_metadata(raw_file_address);
    }
    if (ZWOVideoTextFileInfo::is_valid_zwo_video_text_file(raw_file_address + ".txt")) {
        ZWOVideoTextFileInfo zwo_video_text_file_info(raw_file_address + ".txt");
        Metadata metadata = zwo_video_text_file_info.get_metadata();

        if (metadata.timestamp == 0) {
            Metadata timestamp_metadata = get_file_creation_timestamp(raw_file_address, metadata);
            metadata.timestamp = timestamp_metadata.timestamp;
            metadata.date_time = timestamp_metadata.date_time;
        }

        return metadata;
    }
    return read_metadata_from_raw_file_dslr_slr(raw_file_address);
};

bool AstroPhotoStacker::is_raw_file(const std::string &file_address)   {
    if (AstroPhotoStacker::is_ser_file(file_address)) {
        return true;
    }
    if (ZWOVideoTextFileInfo::is_valid_zwo_video_text_file(file_address + ".txt")) {
        return true;
    }
    if (is_fit_file(file_address)) {
        return true;
    }
    return is_raw_file_dslr_slr(file_address);
};

bool AstroPhotoStacker::get_photo_resolution_raw_file(const std::string &raw_file, int *width, int *height) {
    if (ZWOVideoTextFileInfo::is_valid_zwo_video_text_file(raw_file + ".txt")) {
        read_one_channel_from_video_frame<short>(raw_file, 0, width, height, 0);
        return true;
    }
    if (AstroPhotoStacker::is_ser_file(raw_file)) {
        AstroPhotoStacker::get_ser_file_width_and_height(raw_file, width, height);
        return true;
    }
    if (is_fit_file(raw_file)) {
        FitFileMetadataReader fit_file_metadata_reader(raw_file);
        *width = fit_file_metadata_reader.get_width();
        *height = fit_file_metadata_reader.get_height();
        return true;
    }
    return get_photo_resolution_raw_file_dslr_slr(raw_file, width, height);
};
