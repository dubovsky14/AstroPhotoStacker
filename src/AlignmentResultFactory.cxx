#include "../headers/AlignmentResultFactory.h"

#include "../headers/AlignmentResultSurface.h"
#include "../headers/AlignmentResultPlateSolving.h"
#include "../headers/AlignmentResultDummy.h"
#include "../headers/AlignmentResultTranslationOnly.h"

#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;


AlignmentResultFactory::AlignmentResultFactory()    {
    register_alignment_result_type<AlignmentResultSurface>();
    register_alignment_result_type<AlignmentResultPlateSolving>();
    register_alignment_result_type<AlignmentResultDummy>();
    register_alignment_result_type<AlignmentResultTranslationOnly>();
};


AlignmentResultFactory& AlignmentResultFactory::get_instance() {
    static AlignmentResultFactory instance;
    return instance;
};

std::unique_ptr<AlignmentResultBase> AlignmentResultFactory::create_alignment_result_from_type(const std::string &type_string)  const {
    auto it = m_alignment_result_constructors_empty.find(type_string);
    if (it != m_alignment_result_constructors_empty.end()) {
        return it->second();
    }
    throw runtime_error("Unknown alignment result type: " + type_string);
}

std::unique_ptr<AlignmentResultBase> AlignmentResultFactory::create_alignment_result_from_description_string(const std::string &description_string) const {
    const std::array<std::string, 3> type_and_description = AlignmentResultBase::split_type_and_description_and_score(description_string);
    const std::string &type_string = type_and_description[0];
    const std::string &method_specific_description = type_and_description[1];
    const std::string &score_string = type_and_description[2];

    auto it = m_alignment_result_constructors_description.find(type_string);
    if (it != m_alignment_result_constructors_description.end()) {
        std::unique_ptr<AlignmentResultBase> result = it->second(method_specific_description);
        result->set_frame_score(score_string);
        return result;
    }
    throw runtime_error("Unknown alignment result type: " + type_string);
}