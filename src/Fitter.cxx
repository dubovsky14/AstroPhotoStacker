#include "../headers/Fitter.h"

#include <cmath>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


Fitter::Fitter(std::vector<double> *parameters, const std::vector<std::pair<double,double>> &limits)    {
    m_parameters = parameters;
    m_num_parameters = limits.size();
    if (m_parameters->size() != m_num_parameters) {
        cerr << "Error: parameters and limits have different sizes\n";
        exit(1);
    }

    m_limits = limits;
};

void Fitter::reset_parameters(std::vector<double> *parameters)    {
    m_parameters = parameters;
};

void Fitter::set_limits(const std::vector<std::pair<double,double>> &limits)  {
    m_limits = limits;
};

void Fitter::fit_gradient(std::function<double(const double *parameters)> objective_function, double learning_rate, double decay, unsigned int max_iterations)  {
    double gradient[m_num_parameters];
    double second_derivative[m_num_parameters];
    vector<double> updated_parameters = *m_parameters;
    for (unsigned int i_iter = 0; i_iter < max_iterations; i_iter++) {
        if (m_debug) {
            cout << "Iteration " << i_iter << endl;
        }
        calculate_gradient_and_second_derivative(objective_function, gradient, second_derivative);
        normalize_vector(gradient, m_num_parameters);
        const double nominal_value = objective_function(m_parameters->data());

        updated_parameters = *m_parameters;
        for (unsigned int i_param = 0; i_param < m_num_parameters; i_param++) {
            if (second_derivative[i_param] == 0 || isnan(second_derivative[i_param]) || isnan(gradient[i_param])) {
                continue;
            }
            updated_parameters.at(i_param) -= learning_rate*gradient[i_param];

            if (updated_parameters.at(i_param) < m_limits[i_param].first) {
                updated_parameters.at(i_param) = m_limits[i_param].first;
            }
            if (updated_parameters.at(i_param) > m_limits[i_param].second) {
                updated_parameters.at(i_param) = m_limits[i_param].second;
            }
        }
        const double updated_value = objective_function(updated_parameters.data());
        if (updated_value < nominal_value) {
            *m_parameters = updated_parameters;
            learning_rate /= decay;
        }
        else {
            learning_rate *= decay;
        }
    }
};

void Fitter::calculate_gradient_and_second_derivative(std::function<double(const double *parameters)> objective_function, double *gradient, double *second_derivative)  {
    const double delta = m_gradient_step;
    const double nominal_value = objective_function(m_parameters->data());
    for (unsigned int i_param = 0; i_param < m_num_parameters; i_param++) {
        if (m_limits[i_param].second == m_limits[i_param].first)    {
            gradient[i_param] = 0;
            second_derivative[i_param] = 0;
            continue;
        }
        double parameters_plus_delta[m_num_parameters];
        double parameters_minus_delta[m_num_parameters];
        for (unsigned int j = 0; j < m_num_parameters; j++) {
            parameters_plus_delta[j] = m_parameters->at(j);
            parameters_minus_delta[j] = m_parameters->at(j);
        }

        const double delta_this_parameter = delta*abs(m_limits[i_param].second - m_limits[i_param].first);
        parameters_plus_delta[i_param] += delta_this_parameter;
        parameters_minus_delta[i_param] -= delta_this_parameter;

        const double value_plus = objective_function(parameters_plus_delta);
        const double value_minus = objective_function(parameters_minus_delta);

        if (m_debug)  {
            cout << "\tParameter " << i_param << " " << m_parameters->at(i_param) << " (" << nominal_value << ")"  <<
                "\t" << parameters_plus_delta[i_param] << " (" << value_plus << ")\t"
                "\t" << parameters_minus_delta[i_param] << " (" << value_minus << ")\t"
                 << endl;
        }

        gradient[i_param] = (value_plus - value_minus) / (2*delta_this_parameter);
        second_derivative[i_param] = (value_plus - 2*nominal_value + value_minus) / (delta_this_parameter*delta_this_parameter);
    }
};

void Fitter::normalize_vector(double *vector, unsigned int size) {
    double norm = 0;
    for (unsigned int i = 0; i < size; i++) {
        norm += vector[i]*vector[i];
    }
    norm = sqrt(norm);
    for (unsigned int i = 0; i < size; i++) {
        vector[i] /= norm;
    }
};

void Fitter::set_debug(bool debug)  {
    m_debug = debug;
};


void Fitter::set_gradient_step(double step) {
    m_gradient_step = step;
};