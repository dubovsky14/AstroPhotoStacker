#pragma once

#include "../headers/Common.h"

#include <string>
#include <map>
#include <stdexcept>

namespace AstroPhotoStacker {
    class InputArgumentsParser {
        public:
            InputArgumentsParser() = delete;

            InputArgumentsParser(int argc, const char **argv);

            template<typename return_type>
            return_type get_argument(const std::string &argument_name) const    {
                if (m_arguments.find(argument_name) == m_arguments.end()) {
                    throw std::runtime_error("Input argument not found: " + argument_name);
                }

                return convert_string_to<return_type>(m_arguments.at(argument_name));
            };

            template<typename return_type>
            return_type get_optional_argument(const std::string &argument_name, return_type default_value) const    {
                if (m_arguments.find(argument_name) == m_arguments.end()) {
                    return default_value;
                }

                return convert_string_to<return_type>(m_arguments.at(argument_name));
            };

        private:
            std::map<std::string, std::string> m_arguments;
    };
}