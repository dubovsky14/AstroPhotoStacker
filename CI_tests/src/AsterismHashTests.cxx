#include "../headers/AsterismHashTests.h"

#include "../../headers/AsterismHasher.h"
#include "../../headers/Common.h"

#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;


TestResult AstroPhotoStacker::test_asterism_hasher(const std::vector<std::tuple<float,float,int>> &stars, const std::vector<float> &expected_hash,
                                    unsigned int expected_index_star_A, unsigned int expected_index_star_B,
                                    unsigned int expected_index_star_C, unsigned int expected_index_star_D) {

    string error_message;

    vector<float> result;
    unsigned int index_star_A, index_star_B, index_star_C, index_star_D;
    const bool hash_success = calculate_asterism_hash(stars, &result, &index_star_A, &index_star_B, &index_star_C, &index_star_D);

    if (hash_success != (expected_hash.size() == 4))    {
        const std::string calculation_result = hash_success ?               "valid" : "invalid";
        const std::string expected_result    = expected_hash.size() == 4 ?  "valid" : "invalid";
        return TestResult(false, "Got " + calculation_result + " hash, but expected " + expected_result + " hash");
    }

    for (unsigned int i = 0; i < expected_hash.size(); i++) {
        if (fabs(result[i] - expected_hash[i]) > 1e-3) {
            error_message += "Hash values do not match: ";
            auto hash_to_string = [](const vector<float> &hash) {
                vector<string> numbers_as_strings;
                for (const auto &number : hash) {
                    numbers_as_strings.push_back(to_string(number));
                }
                return "[" + join_strings(", ", numbers_as_strings) + "]";
            };
            error_message += "expected: " + hash_to_string(expected_hash) + ", got " + hash_to_string(result) + "\n";
            break;
        }
    }

    auto check_index_match = [&error_message](unsigned int index, unsigned int expected_index, const string &star_name) {
        if (index != expected_index) {
            error_message += "Index of star " + star_name + " does not match: expected " + to_string(expected_index) + ", got " + to_string(index) + "\n";
        }
    };
    check_index_match(index_star_A, expected_index_star_A, "A");
    check_index_match(index_star_B, expected_index_star_B, "B");
    check_index_match(index_star_C, expected_index_star_C, "C");
    check_index_match(index_star_D, expected_index_star_D, "D");

    if (!error_message.empty()) {
        return TestResult(false, error_message);
    }
    return TestResult(true, "Asterism hash test passed.");
}

