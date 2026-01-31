#pragma once

#include <string>
#include <functional>

namespace AstroPhotoStacker {
    class AdditionalStackerSettingNumerical {
        public:
            AdditionalStackerSettingNumerical() = delete;

            /**
             * @brief Construct a new Additional Stacker Setting object
             *
             * @param name - name of the setting
             * @param value_address - pointer to the variable that holds the value of the setting
             * @param range - valid range for the setting
             * @param step - step size for the setting
            */
            template<typename ValueType>
            AdditionalStackerSettingNumerical(const std::string &name, ValueType* value_address, double min_value, double max_value, double step)
                : m_name(name), m_range(std::make_pair(min_value, max_value)), m_step(step) {
                    m_set_value_function = [value_address, min_value, max_value](double v) {
                        if (v < min_value) {
                            v = min_value;
                        }
                        if (v > max_value) {
                            v = max_value;
                        }
                        *value_address = static_cast<ValueType>(v);
                    };
                    m_get_value_function = [value_address]() {
                        return static_cast<double>(*value_address);
                    };
                    m_default_value = static_cast<double>(*value_address);
            }

            /**
             * @brief Get the name of the setting
             *
             * @return const std::string& - name of the setting
            */
            const std::string& get_name() const { return m_name; };

            /**
             * @brief Get the valid range for the setting
             *
             * @return const std::pair<double, double>& - valid range for the setting
            */
            const std::pair<double, double>& get_range() const { return m_range; };

            /**
             * @brief Get the step size for the setting
             *
             * @return double - step size for the setting
            */
            double get_step() const { return m_step; };

            /**
             * @brief Set the value of the setting
             *
             * @param value - value to set
            */
            void set_value(double value) { m_set_value_function(value); };

            /**
             * @brief Get the value of the setting
             *
             * @return double - current value of the setting
            */
            double get_value() const { return m_get_value_function(); };

            double get_default_value() const { return m_default_value; };

        private:
            std::string m_name;
            std::pair<double, double> m_range;
            double m_step;
            double m_default_value;

            std::function<void(double)> m_set_value_function;
            std::function<double()>     m_get_value_function;
    };
}