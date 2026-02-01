#pragma once

#include "../headers/InputFrame.h"
#include "../headers/AdditionalStackerSettingNumerical.h"
#include "../headers/ConfigurableAlgorithmSettings.h"

#include <string>
#include <thread>
#include <vector>
#include <map>

/**
 * @brief Class responsible for storing settings for the stacking process.
*/
namespace AstroPhotoStacker {
    class StackSettings {
        public:
            StackSettings() = default;
            ~StackSettings() = default;

            void set_alignment_frame(const AstroPhotoStacker::InputFrame& alignment_frame);
            const AstroPhotoStacker::InputFrame& get_alignment_frame() const;

            // alignment method
            void set_alignment_method(const std::string& alignment_method);
            const std::string& get_alignment_method() const;

            // nCPU
            int get_max_threads() const;
            void set_n_cpus(int n_cpus);
            int get_n_cpus() const;

            // max memory
            void set_max_memory(int max_memory);
            int get_max_memory() const;

            // stacking algorithm
            const std::vector<std::string>& get_stacking_algorithms() const;
            void set_stacking_algorithm(const std::string& stacking_algorithm);
            const std::string& get_stacking_algorithm() const;

            // hot pixel correction
            void set_hot_pixel_correction(bool hot_pixel_correction);
            bool use_hot_pixel_correction() const;

            // color interpolation
            void set_use_color_interpolation(bool use_color_interpolation);
            bool use_color_interpolation() const;

            // color stretching
            void set_apply_color_stretching(bool apply_color_stretching);
            bool apply_color_stretching() const;

            // algorithm specific settings
            void clear_algorithm_specific_settings() {
                m_algorithm_specific_settings_numeric.clear();
                m_algorithm_specific_settings_bool.clear();
            };

            void set_algorithm_specific_setting(const std::string& name, double value) {
                m_algorithm_specific_settings_numeric[name] = value;
            };

            void set_algorithm_specific_settings(std::map<std::string, double> settings) {
                m_algorithm_specific_settings_numeric = settings;
            };

            ConfigurableAlgorithmSettingsMap get_algorithm_specific_settings() const    {
                ConfigurableAlgorithmSettingsMap map;
                map.numerical_settings = m_algorithm_specific_settings_numeric;
                map.bool_settings = m_algorithm_specific_settings_bool;
                return map;
            };

            std::vector<AdditionalStackerSettingNumerical> get_algorithm_specific_settings_defaults() const;

        private:
            AstroPhotoStacker::InputFrame m_alignment_frame;
            std::string m_stacking_algorithm = "kappa-sigma mean";
            int m_n_cpus = get_max_threads();
            int m_max_memory = 8000;

            bool  m_hot_pixel_correction = false;
            bool  m_use_color_interpolation = true;
            bool  m_apply_color_stretching = false;

            std::string m_alignment_method = "stars";

            static const std::vector<std::string> m_stacking_algorithms;

            std::map<std::string, double> m_algorithm_specific_settings_numeric;
            std::map<std::string, bool> m_algorithm_specific_settings_bool;

    };
}