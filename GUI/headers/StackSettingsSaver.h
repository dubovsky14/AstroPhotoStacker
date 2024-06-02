#pragma once

#include "../headers/StackSettings.h"

#include <string>
#include <map>
#include <fstream>

/**
 * @brief Class responsible for saving and loading settings, so that they are persistent between sessions.
*/
class StackSettingsSaver : public StackSettings  {
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
        std::string m_dict_string_kappa = "kappa";
        std::string m_dict_string_kappa_sigma_iter = "kappa_sigma_iter";
        std::string m_dict_string_cut_off_tail_fraction = "cut_off_tail_fraction";
        std::string m_dict_string_hot_pixel_correction = "hot_pixel_correction";
        std::string m_dict_string_use_color_interpolation = "use_color_interpolation";
        std::string m_dict_string_apply_color_stretching = "apply_color_stretching";
};