#include "../headers/RawFileReaderFactory.h"
#include "../headers/InputFormatTypes.h"

#include "../headers/RawFileReaderFit.h"
#include "../headers/RawFileReaderVideoSer.h"
#include "../headers/RawFileReaderVideoZWO.h"
#include "../headers/RawFileReaderDSLR.h"

using namespace AstroPhotoStacker;
using namespace std;

std::unique_ptr<RawFileReaderBase> RawFileReaderFactory::get_raw_file_reader(const InputFrame &input_frame) {
    const InputFormatType format = InputFormatTypeGetter::get_input_format_type(input_frame);
    switch (format) {
        case InputFormatType::FIT_FILE:
            return std::make_unique<RawFileReaderFit>(input_frame);
        case InputFormatType::VIDEO_FRAME_RAW_SER:
            return std::make_unique<RawFileReaderVideoSer>(input_frame);
        case InputFormatType::VIDEO_FRAME_RAW_ZWO:
            return std::make_unique<RawFileReaderVideoZWO>(input_frame);
        case InputFormatType::DSLR_OR_SLR_RAW_FILE:
            return std::make_unique<RawFileReaderDSLR>(input_frame);
        default:
            throw std::invalid_argument("Unsupported input format for raw file reader");
    }
}