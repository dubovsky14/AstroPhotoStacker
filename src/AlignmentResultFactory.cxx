#include "../headers/AlignmentResultFactory.h"

#include "../headers/AlignmentResultSurface.h"
#include "../headers/AlignmentResultPlateSolving.h"

#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;


AlignmentResultFactory::AlignmentResultFactory()    {
    register_alignment_result_type<AlignmentResultSurface>();
    register_alignment_result_type<AlignmentResultPlateSolving>();
};


AlignmentResultFactory& AlignmentResultFactory::get_instance() {
    static AlignmentResultFactory instance;
    return instance;
};

std::unique_ptr<AlignmentResultBase> AlignmentResultFactory::create_alignment_result_from_type(const std::string &type_string) {
    auto it = m_alignment_result_constructors_empty.find(type_string);
    if (it != m_alignment_result_constructors_empty.end()) {
        return it->second();
    }
    throw runtime_error("Unknown alignment result type: " + type_string);
}

std::unique_ptr<AlignmentResultBase> AlignmentResultFactory::create_alignment_result_from_description_string(const std::string &description_string) {
    const std::pair<std::string, std::string> type_and_description = AlignmentResultBase::split_type_and_description(description_string);
    const std::string &type_string = type_and_description.first;
    const std::string &method_specific_description = type_and_description.second;

    auto it = m_alignment_result_constructors_description.find(type_string);
    if (it != m_alignment_result_constructors_description.end()) {
        return it->second(method_specific_description);
    }
    throw runtime_error("Unknown alignment result type: " + type_string);
}