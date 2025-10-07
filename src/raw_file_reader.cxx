#include "../headers/raw_file_reader.h"

#include "../headers/MetadataCommon.h"
#include "../headers/Common.h"

#include "../headers/ZWOVideoTextFileInfo.h"    // #TODO: clean up this spaghetti
#include "../headers/RawFileReaderDSLR.h"       // #TODO: clean up this spaghetti

#include <libraw/libraw.h>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace AstroPhotoStacker;

bool AstroPhotoStacker::is_fit_file(const std::string &file_address)   {
    return ends_with(to_upper_copy(file_address), ".FIT");
};

Metadata AstroPhotoStacker::read_metadata_from_raw_file(const std::string &raw_file_address)    {
    InputFrame input_frame(raw_file_address);
    std::unique_ptr<RawFileReaderBase> raw_file_reader = RawFileReaderFactory::get_raw_file_reader(input_frame);
    return raw_file_reader->read_metadata();
};

bool AstroPhotoStacker::get_photo_resolution_raw_file(const std::string &raw_file, int *width, int *height) {
    InputFrame input_frame(raw_file);
    std::unique_ptr<RawFileReaderBase> raw_file_reader = RawFileReaderFactory::get_raw_file_reader(input_frame);
    raw_file_reader->get_photo_resolution(width, height);
    return true;
};
