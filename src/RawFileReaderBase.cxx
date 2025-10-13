#include "../headers/RawFileReaderBase.h"

#include "../headers/CustomSharedMutex.h"

using namespace AstroPhotoStacker;
using namespace std;


RawFileReaderBase::RawFileReaderBase(const InputFrame &input_frame) : FrameReaderBase(input_frame) {
}

