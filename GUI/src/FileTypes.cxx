#include "../headers/FileTypes.h"

std::string to_string(FileTypes type)   {
    switch (type)   {
        case FileTypes::FLAT:
            return "FLAT";
        case FileTypes::LIGHT:
            return "LIGHT";
        case FileTypes::DARK:
            return "DARK";
        case FileTypes::BIAS:
            return "BIAS";
        case FileTypes::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
};

FileTypes string_to_filetype(const std::string& type)   {
    if (type == "FLAT")     {
        return FileTypes::FLAT;
    }
    else if (type == "LIGHT")   {
        return FileTypes::LIGHT;
    }
    else if (type == "DARK")    {
        return FileTypes::DARK;
    }
    else if (type == "BIAS")    {
        return FileTypes::BIAS;
    }
    else    {
        return FileTypes::UNKNOWN;
    }
};