#pragma once

#include <functional>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker {
    class Fitter    {
        public:
            Fitter() = delete;

            Fitter(float *parameters, const std::vector<std::pair<float,float>> &limits);

            void reset_parameters(float *parameters);

            void set_limits(const std::vector<std::pair<float,float>> &limits);

            void fit_gradient(std::function<float(const float *parameters)> objective_function, float learning_rate = 0.1, float decay = 0.999, unsigned int max_iterations = 1000);

            void calculate_gradient_and_second_derivative(std::function<float(const float *parameters)> objective_function, float *gradient, float *second_derivative);

        private:
            float *m_parameters;
            unsigned int m_num_parameters;
            std::vector<std::pair<float,float>> m_limits;

            static void normalize_vector(float *vector, unsigned int size);


    };
}