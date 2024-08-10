#pragma once

#include <functional>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker {
    class Fitter    {
        public:
            Fitter() = delete;

            Fitter(std::vector<double> *parameters, const std::vector<std::pair<double,double>> &limits);

            void reset_parameters(std::vector<double> *parameters);

            void set_limits(const std::vector<std::pair<double,double>> &limits);

            void fit_gradient(std::function<double(const double *parameters)> objective_function, double learning_rate = 0.1, double decay = 0.999, unsigned int max_iterations = 1000);

            void calculate_gradient_and_second_derivative(std::function<double(const double *parameters)> objective_function, double *gradient, double *second_derivative);

            void set_debug(bool debug);

            void set_gradient_step(double step);

        private:
            unsigned int m_num_parameters;
            std::vector<double> *m_parameters = nullptr;
            std::vector<std::pair<double,double>> m_limits;

            static void normalize_vector(double *vector, unsigned int size);

            bool m_debug = false;

            double m_gradient_step = 1e-4;


    };
}