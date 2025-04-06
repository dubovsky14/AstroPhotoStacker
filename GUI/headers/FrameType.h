#pragma once

#include <string>

/**
 * @brief Enum class for file (image) types
*/
enum class FrameType    {
    LIGHT,
    FLAT,
    DARK,
    BIAS,
    UNKNOWN
};

/**
 * @brief Convert FrameType enum to string
 *
 * @param type FrameType enum
 * @return std::string string representation of the FrameType enum
*/
std::string to_string(FrameType type);

/**
 * @brief Convert string to FrameType enum
 *
 * @param type string representation of the FrameType enum
 * @return FrameType FrameType enum
*/
FrameType string_to_filetype(const std::string& type);