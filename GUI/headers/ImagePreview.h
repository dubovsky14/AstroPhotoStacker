#pragma once

#include <vector>
#include <string>

std::vector<std::vector<int>> get_preview(const std::string &path, int width, int height, int *max_value);

std::vector<std::vector<int>> get_preview_from_stacked_picture( const std::vector<std::vector<double>> stacked_image,
                                                                int width_original, int height_original,
                                                                int width_resized, int height_resized,
                                                                int *max_value);