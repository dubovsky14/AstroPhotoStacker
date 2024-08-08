#include "../headers/Fitter.h"

#include <cmath>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


Fitter::Fitter(float *parameters, const std::vector<std::pair<float,float>> &limits)    {
    m_parameters = parameters;
    m_num_parameters = limits.size();
    m_limits = limits;
};

void Fitter::reset_parameters(float *parameters)    {
    m_parameters = parameters;
};

void Fitter::set_limits(const std::vector<std::pair<float,float>> &limits)  {
    m_limits = limits;
};

void Fitter::fit_gradient(std::function<float(const float *parameters)> objective_function, float learning_rate, float decay, unsigned int max_iterations)  {
    float gradient[m_num_parameters];
    float second_derivative[m_num_parameters];
    for (unsigned int i = 0; i < max_iterations; i++) {
        if (i % 100 == 0) {
            cout << "Iteration " << i << endl;
        }
        calculate_gradient_and_second_derivative(objective_function, gradient, second_derivative);
        //normalize_vector(gradient, m_num_parameters);
        for (unsigned int j = 0; j < m_num_parameters; j++) {
            m_parameters[j] -= learning_rate*gradient[j]/second_derivative[j];
        }
        learning_rate *= decay;
    }
};

void Fitter::calculate_gradient_and_second_derivative(std::function<float(const float *parameters)> objective_function, float *gradient, float *second_derivative)  {
    const float delta = 1e-4;
    const float nominal_value = objective_function(m_parameters);
    for (unsigned int i = 0; i < m_num_parameters; i++) {
        float parameters_plus_delta[m_num_parameters];
        float parameters_minus_delta[m_num_parameters];
        for (unsigned int j = 0; j < m_num_parameters; j++) {
            parameters_plus_delta[j] = m_parameters[j];
            parameters_minus_delta[j] = m_parameters[j];
        }
        const float delta_this_parameter = delta*abs(m_limits[i].second - m_limits[i].first);
        parameters_plus_delta[i] += delta_this_parameter;
        parameters_minus_delta[i] -= delta_this_parameter;

        const float value_plus = objective_function(parameters_plus_delta);
        const float value_minus = objective_function(parameters_minus_delta);

        gradient[i] = (value_plus - value_minus) / (2*delta_this_parameter);
        second_derivative[i] = (value_plus - 2*nominal_value + value_minus) / (delta_this_parameter*delta_this_parameter);
    }
};

void Fitter::normalize_vector(float *vector, unsigned int size) {
    float norm = 0;
    for (unsigned int i = 0; i < size; i++) {
        norm += vector[i]*vector[i];
    }
    norm = sqrt(norm);
    for (unsigned int i = 0; i < size; i++) {
        vector[i] /= norm;
    }
};