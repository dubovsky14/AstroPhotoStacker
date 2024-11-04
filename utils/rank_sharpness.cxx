#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/SharpnessRanker.h"

#include "../headers/Fitter.h"

#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <tuple>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    if (argc != 2)   {
        std::cerr << "Usage: rank_sharpness <file>\n";
        return 1;
    }

    const std::string input_file = argv[1];

    // is file?
    const bool is_file = std::filesystem::is_regular_file(input_file);
    if (is_file)    {
        const float sharpness = get_sharpness_for_file(InputFrame(input_file));
        std::cout << sharpness << std::endl;
    }
    else {
        // loop over jpg files:
        vector<tuple<string, float>> sharpness_values;
        for (const auto &entry : std::filesystem::directory_iterator(input_file)) {
            const std::string file = entry.path().string();
            bool is_jpg = file.find(".jpg") != std::string::npos || file.find(".JPG") != std::string::npos;

            if (!is_jpg) {
                continue;
            }
            const float sharpness = get_sharpness_for_file(InputFrame(file));
            sharpness_values.push_back({file, sharpness});
        }
        sort(sharpness_values.begin(), sharpness_values.end(), [](const tuple<string, float> &a, const tuple<string, float> &b) {
            return get<1>(a) > get<1>(b);
        });

        cout << "file\t\tsharpness" << endl;
        for (const auto &entry : sharpness_values) {
            std::cout << get<0>(entry) << " " << get<1>(entry) << std::endl;
        }
    }

}