#include "../headers/ConfigurableAlgorithmSettings.h"

#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;


void ConfigurableAlgorithmSettings::set_values_from_configuration_map(const ConfigurableAlgorithmSettingsMap &configuration_map) {
    configure_with_settings_numerical(configuration_map.numerical_settings);
    configure_with_settings_bool(configuration_map.bool_settings);
};

std::vector<std::string> ConfigurableAlgorithmSettings::get_additional_setting_keys_numerical() const  {
    std::vector<std::string> keys;
    for (const auto &pair : m_additional_settings_numerical) {
        keys.push_back(pair.first);
    }
    return keys;
};

void ConfigurableAlgorithmSettings::set_additional_setting_numerical(const std::string &name, double value)    {
    if (m_additional_settings_numerical.find(name) == m_additional_settings_numerical.end()) {
        throw std::runtime_error("Setting not found: " + name);
    }
    m_additional_settings_numerical.at(name).set_value(value);
};

AdditionalStackerSettingNumerical ConfigurableAlgorithmSettings::get_additional_setting_numerical(const std::string &name) const  {
    if (m_additional_settings_numerical.find(name) != m_additional_settings_numerical.end()) {
        return m_additional_settings_numerical.at(name);
    }
    throw std::runtime_error("Setting not found: " + name);
};


std::vector<std::string> ConfigurableAlgorithmSettings::get_additional_setting_keys_bool() const {
    std::vector<std::string> keys;
    for (const auto &pair : m_additional_settings_bool) {
        keys.push_back(pair.first);
    }
    return keys;
};

AdditionalStackerSettingsBool ConfigurableAlgorithmSettings::get_additional_setting_bool(const std::string &name) const {
    if (m_additional_settings_bool.find(name) != m_additional_settings_bool.end()) {
        return m_additional_settings_bool.at(name);
    }
    throw std::runtime_error("Setting not found: " + name);
};

void ConfigurableAlgorithmSettings::configure_with_settings_bool(std::map<std::string, bool> settings) {
    for (const auto &pair : settings) {
        set_additional_setting_bool(pair.first, pair.second);
    }
};

void ConfigurableAlgorithmSettings::set_additional_setting_bool(const std::string &name, bool value) {
    if (m_additional_settings_bool.find(name) == m_additional_settings_bool.end()) {
        throw std::runtime_error("Setting not found: " + name);
    }
    m_additional_settings_bool.at(name).set_value(value);
};

void ConfigurableAlgorithmSettings::configure_with_settings_numerical(std::map<std::string, double> settings)  {
    for (const auto &pair : settings) {
        set_additional_setting_numerical(pair.first, pair.second);
    }
};

