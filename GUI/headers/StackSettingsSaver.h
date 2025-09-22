#pragma once

#include "../../headers/StackSettings.h"

#include <string>
#include <map>
#include <fstream>

/**
 * @brief Class responsible for saving and loading settings, so that they are persistent between sessions.
*/
class StackSettingsSaver : public AstroPhotoStacker::StackSettings {
    public:
        StackSettingsSaver() = delete;

        StackSettingsSaver(const std::string &filename);

        /**
         * @brief Destructor - save the settings to a file
        */
        ~StackSettingsSaver();

        /**
         * @brief Save the current settings to a file
        */
        void save();

        bool has_algorithm_specific_setting_from_previous_session(const std::string &name) const;

        double get_algorithm_specific_setting_from_previous_session(const std::string &name) const;

        void set_algorithm_specific_setting_from_previous_session(const std::string &name, double value);

    private:
        /**
         * @brief Load the settings from a file - if file is not there, use default settings
        */
        void load();


        std::string m_filename;


        std::string m_separator = "|";
        std::string m_dict_string_n_cpus = "n_cpus";
        std::string m_dict_string_max_memory = "max_memory";
        std::string m_dict_string_stacking_algorithm = "stacking_algorithm";
        std::string m_dict_string_hot_pixel_correction = "hot_pixel_correction";
        std::string m_dict_string_use_color_interpolation = "use_color_interpolation";
        std::string m_dict_string_apply_color_stretching = "apply_color_stretching";


        std::string m_dict_string_algorithm_specific = "algorithm_specific_settings:";
        std::map<std::string, double> m_algorithm_specific_settings_from_previous_session;

};