#include "../headers/Metadata.h"

#include "../headers/MetadataCommon.h"

using namespace AstroPhotoStacker;

std::string Metadata::get_datetime(const std::string &format) const {
    return get_string_timestamp_from_unix_time(timestamp, format);
}