#include "../headers/FileTypes.h"

int file_type_to_index(FileTypes type)  {
    switch (type) {
        case FileTypes::LIGHT: return 0;
        case FileTypes::FLAT:  return 1;
        case FileTypes::DARK:  return 2;
        case FileTypes::BIAS:  return 3;
        default: return -1;
    }
};

//bool operator<(const FileTypes &a, const FileTypes &b) {
//    return file_type_to_index(a) < file_type_to_index(b);
//};
//
//bool operator>(const FileTypes &a, const FileTypes &b) {
//    return file_type_to_index(a) > file_type_to_index(b);
//};
//
//bool operator<=(const FileTypes &a, const FileTypes &b) {
//    return file_type_to_index(a) <= file_type_to_index(b);
//};
//
//bool operator>=(const FileTypes &a, const FileTypes &b) {
//    return file_type_to_index(a) >= file_type_to_index(b);
//};

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