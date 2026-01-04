#include "../headers/SettingsCustomization.h"

#include "../../headers/Common.h"

#include <fstream>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

std::string boolean_map_to_string(const std::map<std::string, bool*> &boolean_map)  {
    std::string settings_string;
    for (const auto &pair : boolean_map) {
        settings_string += pair.first + ": " + ( *(pair.second) ? "1" : "0" ) + ";";
    }
    return settings_string;
};

void set_boolean_map_from_string(const std::string &settings_string, const std::map<std::string, bool*> &boolean_map)  {
    const std::vector<std::string> settings_parts = AstroPhotoStacker::split_string(settings_string, ";");
    for (const std::string &key_value_string : settings_parts) {
        std::vector<std::string> key_value = AstroPhotoStacker::split_and_strip_string(key_value_string, ":");
        if (key_value.size() != 2) {
            continue;
        }
        const std::string &key = key_value[0];
        const std::string &value = key_value[1];
        if (boolean_map.find(key) != boolean_map.end()) {
            bool *setting_pointer = boolean_map.at(key);
            *setting_pointer = (value == "1");
        }
    }
};

std::string  MetadataViewSettings::to_string() {
    return boolean_map_to_string( get_boolean_map() );
};

MetadataViewSettings::MetadataViewSettings(const std::string &settings_string) {
    const std::map<std::string, bool*> boolean_map = get_boolean_map();
    set_boolean_map_from_string(settings_string, boolean_map);
};

std::string FrameStatisticsViewSettings::to_string() {
    return boolean_map_to_string( get_boolean_map() );
};

FrameStatisticsViewSettings::FrameStatisticsViewSettings(const std::string &settings_string) {
    const std::map<std::string, bool*> boolean_map = get_boolean_map();
    set_boolean_map_from_string(settings_string, boolean_map);
};


std::unique_ptr<SettingsCustomization> SettingsCustomization::s_singleton_instance = nullptr;

void SettingsCustomization::initialize_instance(const std::string &settings_text_file_path) {
    if (s_singleton_instance) {
        throw std::runtime_error("SettingsCustomization singleton instance is already initialized.");
    }
    s_singleton_instance = std::unique_ptr<SettingsCustomization>(new SettingsCustomization(settings_text_file_path));
};

SettingsCustomization::SettingsCustomization(const std::string &settings_text_file_path) : m_settings_text_file_path(settings_text_file_path) {
    ifstream input_file (m_settings_text_file_path);
    if (input_file.is_open())    {
        std::string line;
        while ( getline (input_file,line) )        {
            strip_string(&line);
            if (line.length() == 0) {
                continue;
            }

            const std::vector<std::string> parts = split_and_strip_string(line, "|");
            if (parts.size() != 2) {
                continue;
            }

            const std::string &setting_name = parts[0];
            const std::string &setting_value = parts[1];

            if (setting_name == "MetadataViewSettings") {
                metadata_view_settings = MetadataViewSettings(setting_value);
            }
            else if (setting_name == "FrameStatisticsViewSettings") {
                frame_statistics_view_settings = FrameStatisticsViewSettings(setting_value);
            }
        }
        input_file.close();
    }
}

SettingsCustomization::~SettingsCustomization() {
    ofstream output_file(m_settings_text_file_path);
    if (output_file.is_open())    {
        output_file << "MetadataViewSettings | " << metadata_view_settings.to_string() << std::endl;
        output_file << "FrameStatisticsViewSettings | " << frame_statistics_view_settings.to_string() << std::endl;
    }
    output_file.close();
}