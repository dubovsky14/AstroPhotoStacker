#pragma once

#include "../headers/Common.h"

#include <string>


namespace AstroPhotoStacker {
    struct FrameScore {
            float stars_excentricity    = -1;
            float stars_fwhm            = -1;
            float sharpness_score       = -1;

            float brightness_mean   = -1;
            float brightness_std    = -1;
            float brightness_min    = -1;
            float brightness_max    = -1;

            std::string to_string() const  {
                return summary_string_for_variable("stars_excentricity", stars_excentricity) +
                       summary_string_for_variable("stars_fwhm", stars_fwhm) +
                       summary_string_for_variable("sharpness_score", sharpness_score) +
                       summary_string_for_variable("brightness_mean", brightness_mean) +
                       summary_string_for_variable("brightness_std", brightness_std) +
                       summary_string_for_variable("brightness_min", brightness_min) +
                       summary_string_for_variable("brightness_max", brightness_max);
            };

            FrameScore(const std::string& summary_string)   {
                const std::vector<std::string> key_value_pairs = split_string(summary_string, ",");

                // for backward compatibility:
                if (key_value_pairs.size() == 1) {
                    if (key_value_pairs[0].find(":") == std::string::npos) {
                        if (string_is_float(key_value_pairs[0])) {
                            stars_excentricity = std::stof(key_value_pairs[0]);
                        }
                    }
                }

                // new format
                for (const std::string &key_value_pair : key_value_pairs) {
                    const std::vector<std::string> key_and_value = split_string(key_value_pair, ":");
                    if (key_and_value.size() != 2) {
                        continue;
                    }
                    const std::string &key = key_and_value[0];
                    const float value = std::stof(key_and_value[1]);
                    if (key == "stars_excentricity") {
                        stars_excentricity = value;
                    } else if (key == "stars_fwhm") {
                        stars_fwhm = value;
                    } else if (key == "sharpness_score") {
                        sharpness_score = value;
                    } else if (key == "brightness_mean") {
                        brightness_mean = value;
                    } else if (key == "brightness_std") {
                        brightness_std = value;
                    } else if (key == "brightness_min") {
                        brightness_min = value;
                    } else if (key == "brightness_max") {
                        brightness_max = value;
                    }
                }
            };

            FrameScore() = default;

        private:
            static std::string summary_string_for_variable(const std::string &variable_name, float variable_value) {
                if (variable_value < 0) {
                    return "";
                }
                return variable_name + ": " + std::to_string(variable_value) + ",";
            };

    };
}