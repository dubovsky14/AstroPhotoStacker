#include "../headers/FrameType.h"

using namespace AstroPhotoStacker;

std::string AstroPhotoStacker::to_string(FrameType type)   {
    switch (type)   {
        case FrameType::FLAT:
            return "FLAT";
        case FrameType::LIGHT:
            return "LIGHT";
        case FrameType::DARK:
            return "DARK";
        case FrameType::BIAS:
            return "BIAS";
        case FrameType::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
};

FrameType AstroPhotoStacker::string_to_filetype(const std::string& type)   {
    if (type == "FLAT")     {
        return FrameType::FLAT;
    }
    else if (type == "LIGHT")   {
        return FrameType::LIGHT;
    }
    else if (type == "DARK")    {
        return FrameType::DARK;
    }
    else if (type == "BIAS")    {
        return FrameType::BIAS;
    }
    else    {
        return FrameType::UNKNOWN;
    }
};