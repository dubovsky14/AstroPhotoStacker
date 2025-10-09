#include "../headers/NonRawFrameReaderFactory.h"
#include "../headers/InputFormatTypes.h"

#include "../headers/NonRawFrameReaderStillImage.h"
#include "../headers/NonRawFrameReaderVideo.h"

using namespace AstroPhotoStacker;
using namespace std;

std::unique_ptr<NonRawFrameReaderBase> NonRawFrameReaderFactory::get_non_raw_frame_reader(const InputFrame &input_frame) {
    const InputFormatType format = InputFormatTypeGetter::get_input_format_type(input_frame);
    switch (format) {
        case InputFormatType::RGB_FILE:
            return std::make_unique<NonRawFrameReaderStillImage>(input_frame);
        case InputFormatType::VIDEO_FRAME_RGB:
            return std::make_unique<NonRawFrameReaderVideo>(input_frame);
        default:
            throw std::runtime_error("Unsupported non-raw format type");
    }
};