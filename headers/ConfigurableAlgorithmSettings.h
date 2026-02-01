#pragma once

#include "../headers/AdditionalStackerSettingNumerical.h"
#include "../headers/AdditionalStackerSettingsBool.h"

#include <string>
#include <vector>
#include <map>

namespace AstroPhotoStacker {
    struct ConfigurableAlgorithmSettingsMap  {
        std::map<std::string, double> numerical_settings;
        std::map<std::string, bool>   bool_settings;

        std::string to_string() const {
            std::string result = "Numerical settings:\n";
            for (const auto &pair : numerical_settings) {
                result += "  " + pair.first + ": " + std::to_string(pair.second) + "\n";
            }
            result += "Boolean settings:\n";
            for (const auto &pair : bool_settings) {
                result += "  " + pair.first + ": " + (pair.second ? "true" : "false") + "\n";
            }
            return result;
        }
    };


    /**
     * @brief Settings for configurable algorithms
    */
    class ConfigurableAlgorithmSettings {

        public:
            ConfigurableAlgorithmSettings() = default;

            ~ConfigurableAlgorithmSettings() = default;


            /**
             * @brief Copy values from another instance of ConfigurableAlgorithmSettings
             */
            void set_values_from_configuration_map(const ConfigurableAlgorithmSettingsMap &configuration_map);


            ////////////////////////////////////////////
            /// Additional settings - numerical type ///
            ////////////////////////////////////////////

            template<typename ValueType>
            void add_additional_setting_numerical(const std::string &name, ValueType *default_value, double range_min, double range_max, double step) {
                // there is no default constructor for AdditionalStackerSettingNumerical, so we need this monstrosity with "at" and "insert" ...
                if (m_additional_settings_numerical.find(name) != m_additional_settings_numerical.end()) {
                    m_additional_settings_numerical.at(name) = AdditionalStackerSettingNumerical(name, default_value, range_min, range_max, step);
                }
                else {
                    m_additional_settings_numerical.insert({name, AdditionalStackerSettingNumerical(name, default_value, range_min, range_max, step)});
                }
            };

            std::vector<std::string> get_additional_setting_keys_numerical() const;

            void set_additional_setting_numerical(const std::string &name, double value);

            AdditionalStackerSettingNumerical get_additional_setting_numerical(const std::string &name) const;


            ///////////////////////////////////////
            /// Additional settings - bool type ///
            ///////////////////////////////////////

            void add_additional_setting_bool(const std::string &name, bool *default_value) {
                if (m_additional_settings_bool.find(name) != m_additional_settings_bool.end()) {
                    m_additional_settings_bool.at(name) = AdditionalStackerSettingsBool(name, default_value);
                }
                else {
                    m_additional_settings_bool.insert({name, AdditionalStackerSettingsBool(name, default_value)});
                }
            };

            std::vector<std::string> get_additional_setting_keys_bool() const;

            AdditionalStackerSettingsBool get_additional_setting_bool(const std::string &name) const;

        private:

            std::map<std::string, AdditionalStackerSettingNumerical> m_additional_settings_numerical;

            std::map<std::string, AdditionalStackerSettingsBool> m_additional_settings_bool;

            void set_additional_setting_bool(const std::string &name, bool value);

            void configure_with_settings_bool(std::map<std::string, bool> settings);

            void configure_with_settings_numerical(std::map<std::string, double> settings);
    };
}