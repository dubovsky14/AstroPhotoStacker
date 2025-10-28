#pragma once

#include "../headers/AlignmentResultBase.h"

#include <memory>
#include <string>
#include <map>
#include <functional>

namespace AstroPhotoStacker {
    class AlignmentResultFactory {
        public:
            static AlignmentResultFactory& get_instance();

            std::unique_ptr<AlignmentResultBase> create_alignment_result_from_type(const std::string &type_string);

            std::unique_ptr<AlignmentResultBase> create_alignment_result_from_description_string(const std::string &description_string);

        private:
            AlignmentResultFactory();

            AlignmentResultFactory(const AlignmentResultFactory &other) = delete;
            AlignmentResultFactory(AlignmentResultFactory &&other) = delete;
            AlignmentResultFactory& operator=(const AlignmentResultFactory &other) = delete;

            template<typename AlignmentResultType>
            void register_alignment_result_type()   {
                m_alignment_result_constructors_empty[AlignmentResultType::s_type_name] = []() {
                    return std::make_unique<AlignmentResultType>();
                };
                m_alignment_result_constructors_description[AlignmentResultType::s_type_name] = [](const std::string &description_string) {
                    return std::make_unique<AlignmentResultType>(description_string);
                };
            };

            std::map<std::string, std::function<std::unique_ptr<AlignmentResultBase>()>>                    m_alignment_result_constructors_empty;
            std::map<std::string, std::function<std::unique_ptr<AlignmentResultBase>(const std::string &)>> m_alignment_result_constructors_description;
    };

}