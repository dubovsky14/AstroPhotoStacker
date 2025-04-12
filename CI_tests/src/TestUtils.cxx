#include "../headers/TestUtils.h"

#include <iostream>


using namespace AstroPhotoStacker;


void TestRunner::summarize_tests()   const {
    int passed_count = 0;
    int failed_count = 0;
    for (const std::pair<std::string,TestResult> &name_and_result : m_test_results) {
        if (name_and_result.second.passed) {
            passed_count++;
        } else {
            failed_count++;
        }
    }
    std::cout << "Test Results:" << std::endl;
    std::cout << "Tests passed: " << passed_count << std::endl;
    std::cout << "Tests failed: " << failed_count << std::endl;
    std::cout << "\n\n\n";

    for (const std::pair<std::string,TestResult> &name_and_result : m_test_results) {
        const std::string &test_name = name_and_result.first;
        const TestResult &result = name_and_result.second;
        if (!result.passed) {
            std::cout << "Test: "           << test_name << " - Failed" << std::endl;
            std::cout << "Error message:\n" << result.error_message << std::endl;
        }
    }

    if (failed_count == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << "Some tests failed." << std::endl;
        abort();
    }
};
