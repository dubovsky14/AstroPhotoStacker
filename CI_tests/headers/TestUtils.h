#pragma once

#include <string>
#include <vector>
#include <functional>

namespace AstroPhotoStacker {

    struct TestResult {
        bool        passed;
        std::string error_message;

        TestResult(bool result, const std::string &message = "")    {
            passed = result;
            error_message = message;
        };
    };

    class TestRunner {
        public:
            template<typename Func, typename... Args>
            void run_test(const std::string &test_name, const Func &test_function, Args &&...args) {
                try {
                    const TestResult result = test_function(std::forward<Args>(args)...);
                    m_test_results.emplace_back(test_name, result);
                } catch (const std::exception &e) {
                    m_test_results.emplace_back(test_name, TestResult(false, e.what()));
                }
                catch (...) {
                    m_test_results.emplace_back(test_name, TestResult(false, "Unknown exception occurred"));
                }
            };

            void summarize_tests() const;

        private:
            std::vector<std::pair<std::string,TestResult>> m_test_results;
    };
}