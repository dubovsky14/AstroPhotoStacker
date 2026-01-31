#pragma once

#include <string>

namespace AstroPhotoStacker {
    class AdditionalStackerSettingsBool {
        public:
            AdditionalStackerSettingsBool() = delete;


            AdditionalStackerSettingsBool(const std::string &name, bool* value_address) {
                m_name = name;
                m_value_address = value_address;
                m_default_value = *value_address;
            }

            /**
             * @brief Get the name of the setting
             *
             * @return const std::string& - name of the setting
            */
            const std::string& get_name() const { return m_name; };

            /**
             * @brief Set the value of the setting
             *
             * @param value - new value for the setting
            */
            void set_value(bool value) { *m_value_address = value; };

            /**
             * @brief Get the value of the setting
             *
             * @return bool - current value of the setting
            */
            bool get_value() const { return *m_value_address; };

            /**
             * @brief Get the default value of the setting
             *
             * @return bool - default value of the setting
            */
            bool get_default_value() const { return m_default_value; };

        private:
            std::string m_name;

            bool    m_default_value;
            bool*   m_value_address = nullptr;
    };
}