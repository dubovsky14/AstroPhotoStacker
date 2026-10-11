#include "../headers/YamlParserTest.h"

#include "../../headers/YamlParser.h"

#include <string>
#include <cmath>

using namespace std;

namespace AstroPhotoStacker {

    TestResult test_yaml_parser() {
        const std::string input_file_addres = "AstroPhotoStacker_test_files/yaml_test/test_file.yaml";

        YamlNode yaml = load_yaml_file(input_file_addres);

        string error_message;
        if (yaml["catalogues"][0]["name"].get_value<std::string>() != "Messier") {
            error_message += "yaml[\"catalogues\"][0][\"name\"] != \"Messier\"\n";
        }

        if (yaml["catalogues"][0]["objects"][0]["number"].get_value<int>() != 45) {
            error_message += "yaml[\"catalogues\"][0][\"objects\"][0][\"number\"] != 45\n";
        }

        if (std::fabs(yaml["catalogues"][0]["objects"][0]["magnitude"].get_value<float>() - 1.6) > 1e-4) {
            error_message += "yaml[\"catalogues\"][0][\"objects\"][0][\"magnitude\"] != 1.6\n";
        }

        if (yaml["catalogues"][0]["objects"][2]["common_reference"].get_value<string>() != "Andromeda galaxy") {
            error_message += "yaml[\"catalogues\"][0][\"objects\"][2][\"common_reference\"] == \"Andromeda galaxy\"\n";
        }

        if (std::fabs(yaml["catalogues"][0]["objects"][2]["distance"].get_value<float>() - 2500000) > 1) {
            error_message += "yaml[\"catalogues\"][0][\"objects\"][2][\"distance\"] != 2500000\n";
        }

        if (std::fabs(yaml["catalogues"][1]["objects"][1]["magnitude"].get_value<float>() - 0.61) > 1e-4) {
            error_message += "yaml[\"catalogues\"][1][\"objects\"][1][\"magnitude\"] != 0.61\n";
        }

        if (yaml["catalogues"][1]["objects"][1]["photographed"].get_value<bool>() != false) {
            error_message += "yaml[\"catalogues\"][1][\"objects\"][1][\"photographed\"] != false\n";
        }

        if (yaml["my_equipment"]["lenses"][4]["has_terrible_chromatic_abberations"].get_value<bool>() != true) {
            error_message += "yaml[\"my_equipment\"][\"lenses\"][4][\"has_terrible_chromatic_abberations\"] != true\n";
        }

        if (yaml["one_more_element"][0].get_value<string>() != "bla") {
            error_message += "yaml[\"one_more_element\"][0] != \"bla\"\n";
        }

        if (yaml["one_more_element"][1].get_value<string>() != "bla1") {
            error_message += "yaml[\"one_more_element\"][1] != \"bla1\"\n";
        }

        if (yaml["lets_try_inline_list"][2].get_value<int>() != 4) {
            error_message += "yaml[\"lets_try_inline_list\"][2] != 4\n";
        }

        // now let's test default values
        if (yaml["one_more_element"].get_value_at_index<string>(2, "unused default") != "bla2") {
            error_message += "yaml[\"one_more_element\"].get_value_at_index<string>(2, \"unused default\") != \"bla2\"\n";
        }

        if (yaml["one_more_element"].get_value_at_index<string>(3, "used default") != "used default") {
            error_message += "yaml[\"one_more_element\"].get_value_at_index<string>(3, \"used default\") != \"used default\"\n";
        }

        if (yaml.get_value_at_key<bool>("still_eager_to_add_more_to_this_yaml", true) != false) {
            error_message += "yaml.get_value_at_key<bool>(\"still_eager_to_add_more_to_this_yaml\", true) != false\n";
        }



        if (error_message.empty()) {
            return TestResult(true, "Yaml parser test passed.");
        } else {
            return TestResult(false, error_message);
        }


    }
}