#include "../headers/LightPollutionGradientFunctions.h"


using namespace AstroPhotoStacker;

std::map<std::string, std::function<std::unique_ptr<LightPollutionGradientBase>(int, int)>> GradientFunctionsFactory::s_factory_map = {
    {"polynomial2n", [](int width, int height) { return std::make_unique<LightPollutionGradientPolynomial2N>(width, height); }},
    {"polynomial3n", [](int width, int height) { return std::make_unique<LightPollutionGradientPolynomial3N>(width, height); }},
    {"polynomial4n", [](int width, int height) { return std::make_unique<LightPollutionGradientPolynomial4N>(width, height); }}
};
