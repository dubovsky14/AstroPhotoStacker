#include "../headers/LightPollutionRemovalTool.h"

#include "../headers/LightPollutionGradientFunctions.h"

using namespace AstroPhotoStacker;
using namespace std;


std::unique_ptr<LightPollutionGradientBase> AstroPhotoStacker::fit_gradient(const std::vector<std::pair<double, double>> &coordinates, const std::vector<double> &integrated_value, const std::string &function_type) {
    std::unique_ptr<LightPollutionGradientBase> result = get_gradient_function(function_type);

    double learning_rate = 0.01;
    const int    max_iterations = 1000;
    const double decay_rate = 0.99;

    const size_t n_params = result->get_parameters().size();
    for (int iteration = 0; iteration < max_iterations; iteration++) {
        std::vector<double> gradient(n_params, 0.0);
        for (size_t i_sample = 0; i_sample < coordinates.size(); i_sample++) {
            const std::pair<double, double>& coord = coordinates[i_sample];
            const double predicted_value = result->get_value(coord.first, coord.second);
            const double error = predicted_value - integrated_value[i_sample];
            const auto derivative = result->get_derivative(coord.first, coord.second);
            for (size_t i_param = 0; i_param < gradient.size(); i_param++) {
                gradient[i_param] += error * derivative[i_param];
            }
        }
        for (size_t i_param = 0; i_param < gradient.size(); i_param++) {
            gradient[i_param] /= coordinates.size(); // Average gradient
        }
        result->update_parameters(gradient, (-1) * learning_rate);
        learning_rate *= decay_rate;
    }
    return result;
}