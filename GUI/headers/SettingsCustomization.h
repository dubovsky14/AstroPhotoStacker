#pragma once

#include "../../headers/Common.h"

#include <string>
#include <vector>
#include <map>


std::string boolean_map_to_string(const std::map<std::string, bool*> &boolean_map);

void        set_boolean_map_from_string(const std::string &settings_string, const std::map<std::string, bool*> &boolean_map);

struct MetadataViewSettings {
    bool show_exposure_time = true;
    bool show_iso           = true;
    bool show_aperture      = true;
    bool show_temperature   = true;
    bool show_focal_length  = false;
    bool show_resolution    = false;

    std::map<std::string, bool*> get_boolean_map() {
        std::map<std::string, bool*> result;
        result["show_exposure_time"] =  &show_exposure_time;
        result["show_iso"] =            &show_iso;
        result["show_aperture"] =       &show_aperture;
        result["show_temperature"] =    &show_temperature;
        result["show_focal_length"] =   &show_focal_length;
        result["show_resolution"] =     &show_resolution;
        return result;
    };

    std::string to_string();

    MetadataViewSettings() = default;

    explicit MetadataViewSettings(const std::string &settings_string);
};

struct FrameStatisticsViewSettings {
    bool show_mean          = true;
    bool show_stddev        = true;
    bool show_min           = false;
    bool show_max           = false;

    std::map<std::string, bool*> get_boolean_map() {
        std::map<std::string, bool*> result;
        result["show_mean"] =    &show_mean;
        result["show_stddev"] =  &show_stddev;
        result["show_min"] =     &show_min;
        result["show_max"] =     &show_max;
        return result;
    };

    std::string to_string();

    FrameStatisticsViewSettings() = default;

    explicit FrameStatisticsViewSettings(const std::string &settings_string);
};

class SettingsCustomization {
    public:
        MetadataViewSettings        metadata_view_settings;
        FrameStatisticsViewSettings frame_statistics_view_settings;

        static SettingsCustomization &get_instance() {
            static SettingsCustomization instance;
            return instance;
        };



    private:

        SettingsCustomization() = default;
        ~SettingsCustomization() = default;
        SettingsCustomization(SettingsCustomization&&) = default;
        SettingsCustomization(const SettingsCustomization&) = delete;
        SettingsCustomization& operator=(const SettingsCustomization&) = delete;

};