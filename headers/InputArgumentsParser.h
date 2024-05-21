#pragma once

#include "../headers/Common.h"

#include <string>
#include <map>
#include <stdexcept>

namespace AstroPhotoStacker {

    /**
     * @brief A class to parse command line input arguments in a form of -argument_name argument_value
     */
    class InputArgumentsParser {
        public:
            InputArgumentsParser() = delete;

            /**
             *  @brief Constructor
             *
             * @param argc The number of arguments
             * @param argv The arguments
            */
            InputArgumentsParser(int argc, const char **argv);

            /**
             * @brief Get the value of an argument
             *
             * @tparam return_type The type of the return value
             * @param argument_name The name of the argument
            */
            template<typename return_type>
            return_type get_argument(const std::string &argument_name) const    {
                if (m_arguments.find(argument_name) == m_arguments.end()) {
                    throw std::runtime_error("Input argument not found: " + argument_name);
                }

                return convert_string_to<return_type>(m_arguments.at(argument_name));
            };

            /**
             * @brief Get the value of an argument or a default value if the argument is not found
             *
             * @tparam return_type The type of the return value
             * @param argument_name The name of the argument
             * @param default_value The default value
            */
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