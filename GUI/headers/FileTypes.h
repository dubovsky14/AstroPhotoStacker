#pragma once

#include <string>

/**
 * @brief Enum class for file (image) types
*/
enum class FileTypes    {
    FLAT,
    LIGHT,
    DARK,
    BIAS,
    UNKNOWN
};

int file_type_to_index(FileTypes type);

//bool operator<(const FileTypes &a, const FileTypes &b);

//bool operator>(const FileTypes &a, const FileTypes &b);

//bool operator<=(const FileTypes &a, const FileTypes &b);

//bool operator>=(const FileTypes &a, const FileTypes &b);


/**
 * @brief Convert FileTypes enum to string
 *
 * @param type FileTypes enum
 * @return std::string string representation of the FileTypes enum
*/
std::string to_string(FileTypes type);

/**
 * @brief Convert string to FileTypes enum
 *
 * @param type string representation of the FileTypes enum
 * @return FileTypes FileTypes enum
*/
FileTypes string_to_filetype(const std::string& type);