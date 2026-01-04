#pragma once

#include "../../headers/Common.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

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

struct OtherSettingsCustomization {
    bool show_full_frame_paths = true;

    std::map<std::string, bool*> get_boolean_map() {
        std::map<std::string, bool*> result;
        result["show_full_frame_paths"] = &show_full_frame_paths;
        return result;
    };

    std::string to_string();

    OtherSettingsCustomization() = default;

    explicit OtherSettingsCustomization(const std::string &settings_string);
};

class SettingsCustomization {
    public:
        MetadataViewSettings        metadata_view_settings;
        FrameStatisticsViewSettings frame_statistics_view_settings;
        OtherSettingsCustomization   other_settings_customization;

        static void initialize_instance(const std::string &settings_text_file_path);

        static SettingsCustomization &get_instance() {
            if (!s_singleton_instance) {
                throw std::runtime_error("SettingsCustomization singleton instance is not initialized.");
            }
            return *s_singleton_instance;
        };

        ~SettingsCustomization();

    private:

        static std::unique_ptr<SettingsCustomization> s_singleton_instance;

        std::string m_settings_text_file_path = "";

        SettingsCustomization() = delete;
        explicit SettingsCustomization(const std::string &settings_text_file_path);
        SettingsCustomization(SettingsCustomization&&) = default;
        SettingsCustomization(const SettingsCustomization&) = delete;
        SettingsCustomization& operator=(const SettingsCustomization&) = delete;

};