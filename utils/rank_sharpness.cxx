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

float get_sharpness_for_file(const std::string input_file)  {
    CalibratedPhotoHandler calibrated_photo_handler(input_file, true);
    calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
    calibrated_photo_handler.calibrate();

    const std::vector<std::vector<short unsigned int>> &data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();
    const int width = calibrated_photo_handler.get_width();
    const int height = calibrated_photo_handler.get_height();

    float average_sharpness = 0;
    for (unsigned int i_color = 0; i_color < data.size(); i_color++)    {
        const float sharpness = AstroPhotoStacker::get_sharpness_factor(data[i_color].data(), width, height);
        average_sharpness += sharpness;
    }
    average_sharpness /= data.size();

    return average_sharpness;
}

int main(int argc, const char **argv)   {
    vector<float> x{-2,-1,0,1,2};
    //vector<float> x{0,1,2,3,4};
    vector<float> y{-1, 2.1, 3.4, 2.4, -0.9};
    vector<float> param{0,0,0};
    vector<pair<float, float>> limits{{-10, 10}, {-10, 10}, {-10, 10}};

    auto quadratic = [](const float *parameters, float x) {
        return parameters[0] * x * x + parameters[1] * x + parameters[2];
    };

    auto objective_function = [&x, &y, &quadratic](const float *parameters) {
        float sum = 0;
        for (unsigned int i = 0; i < x.size(); i++) {
            const float y_fitted = quadratic(parameters, x[i]);
            const float diff = y_fitted - y[i];
            sum += diff * diff;
        }
        return sum;
    };

    Fitter fitter(param.data(), limits);
    fitter.fit_gradient(objective_function, 0.2, 1, 1000);

    cout << "Fitted parameters: ";
    for (const auto &p : param) {
        cout << p << " ";
    }
    cout << endl;

    cout << "Values:\n";
    for (unsigned int i = 0; i < x.size(); i++) {
        cout << x[i] << "\t" << y[i] << "\t" << quadratic(param.data(), x[i]) << endl;
    }

    return 0;


    if (argc != 2)   {
        std::cerr << "Usage: rank_sharpness <file>\n";
        return 1;
    }

    const std::string input_file = argv[1];

    // is file?
    const bool is_file = std::filesystem::is_regular_file(input_file);
    if (is_file)    {
        const float sharpness = get_sharpness_for_file(input_file);
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
            const float sharpness = get_sharpness_for_file(file);
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