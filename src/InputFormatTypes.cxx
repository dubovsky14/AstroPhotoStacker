#include "../headers/InputFormatTypes.h"

#include "../headers/RawFileReaderDSLR.h"
#include "../headers/RawFileReaderVideoSer.h"
#include "../headers/RawFileReaderFit.h"
#include "../headers/VideoReader.h"
#include "../headers/ZWOVideoTextFileInfo.h"


#include <opencv2/imgcodecs.hpp>


using namespace AstroPhotoStacker;
using namespace std;



ThreadSafeCacheSystem<std::string, InputFormatType> InputFormatTypeGetter::s_format_type_cache;

// TODO: caching of the results to avoid multiple checks for the same file
InputFormatType InputFormatTypeGetter::get_input_format_type(const InputFrame &input_frame)   {
    const string file_address = input_frame.get_file_address();
    return get_input_format_type(file_address);
};

InputFormatType InputFormatTypeGetter::get_input_format_type(const std::string &input_address) {
    auto get_format_type_lambda = [&input_address]() {
        return get_input_format_type_without_cache(input_address);
    };

    InputFormatType format_type = s_format_type_cache.get(input_address, get_format_type_lambda);
    return format_type;
};


InputFormatType InputFormatTypeGetter::get_input_format_type_without_cache(const std::string &file_address) {
    if (is_ser_file(file_address)) {
        return InputFormatType::VIDEO_FRAME_RAW_SER;
    }
    if (ZWOVideoTextFileInfo::is_valid_zwo_video_text_file(file_address + ".txt")) {
        return InputFormatType::VIDEO_FRAME_RAW_ZWO;
    }
    if (is_valid_video_file(file_address)) {
        return InputFormatType::VIDEO_FRAME_RGB;
    }
    if (is_raw_file_dslr_slr(file_address)) {
        return InputFormatType::DSLR_OR_SLR_RAW_FILE;
    }
    if (is_fit_file(file_address)) {
        return InputFormatType::FIT_FILE;
    }
    if (cv::haveImageReader(file_address)) {
        return InputFormatType::RGB_FILE;
    }
    return InputFormatType::NOT_SUPPORTED;
};

bool AstroPhotoStacker::is_raw_file(const std::string &file_address) {
    const InputFormatType input_type = InputFormatTypeGetter::get_input_format_type(file_address);
    return  input_type == InputFormatType::DSLR_OR_SLR_RAW_FILE ||
            input_type == InputFormatType::VIDEO_FRAME_RAW_ZWO ||
            input_type == InputFormatType::VIDEO_FRAME_RAW_SER ||
            input_type == InputFormatType::FIT_FILE;
};