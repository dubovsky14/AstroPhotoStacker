#include "../headers/Metadata.h"

#include "../headers/MetadataCommon.h"

using namespace AstroPhotoStacker;

std::string Metadata::get_datetime() const {
    return get_string_timestamp_from_unix_time(timestamp, "%Y-%m-%d %H:%M:%S");
}