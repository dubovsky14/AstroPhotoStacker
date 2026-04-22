#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <random>
#include <map>
#include <utility>
#include <string>
#include <functional>


namespace AstroPhotoStacker {

    template <typename T>
    void increment_vector(std::vector<T> *target, const std::vector<T> &delta, double learning_rate = 1.0) {
        if (target->size() != delta.size()) {
            throw std::invalid_argument("Target and delta vectors must be of the same size");
        }
        for (size_t i = 0; i < target->size(); ++i) {
            (*target)[i] += learning_rate * delta[i];
        }
    }

    class LightPollutionGradientBase {
        public:
            LightPollutionGradientBase() = delete;

            LightPollutionGradientBase(int width, int height) {
                m_width = width;
                m_height = height;
            };

            LightPollutionGradientBase(const LightPollutionGradientBase&) = default;

            virtual std::unique_ptr<LightPollutionGradientBase> clone() const = 0;

            virtual double get_value(double x, double y) const = 0;

            virtual std::vector<double> get_derivative(double x, double y) const = 0;

            virtual std::vector<double> get_parameters() const {
                return m_parameters;
            };

            virtual void set_parameters(const std::vector<double>& params) = 0;

            virtual void update_parameters(const std::vector<double> &delta, double learning_rate) {
                increment_vector(&m_parameters, delta, learning_rate);
            };

            void normalize_coordinates(double *x, double *y) const {
                *x = *x / m_width;
                *y = *y / m_height;
            };

        protected:
            void initialize_parameters(size_t n_parameters) {
                m_parameters.resize(n_parameters, 0.0);
                std::random_device rd{};
                std::mt19937 gen{rd()};
                std::normal_distribution<> d{0, 1};
                for (size_t i = 0; i < n_parameters; i++) {
                    m_parameters[i] = d(gen);
                }
            }

            std::vector<double> m_parameters;
            double m_width;
            double m_height;

    };

    // y = a[0] + a[1]*x + a[2]*y + a[3]*x^2 + a[4]*x*y + a[5]*y^2
    class LightPollutionGradientPolynomial2N : public LightPollutionGradientBase {
        public:
            LightPollutionGradientPolynomial2N(int width, int height) : LightPollutionGradientBase(width, height) {
                initialize_parameters(6); // Initialize with 6 parameters
            };

            LightPollutionGradientPolynomial2N(const LightPollutionGradientPolynomial2N &other) : LightPollutionGradientBase(other) {};

            virtual std::unique_ptr<LightPollutionGradientBase> clone() const override {
                auto clone = std::make_unique<LightPollutionGradientPolynomial2N>(*this);
                return clone;
            };

            double get_value(double x, double y) const override {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return a[0] + a[1]*x + a[2]*y + a[3]*x*x + a[4]*x*y + a[5]*y*y;
            };

            std::vector<double> get_derivative(double x, double y) const override   {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return {1.0, x, y, x*x, x*y, y*y};
            };

            void set_parameters(const std::vector<double>& params) override {
                if (params.size() != 6) {
                    throw std::invalid_argument("Expected 6 parameters for LightPollutionGradientPolynomial2N");
                }
                m_parameters = params;
            };
    };

    // y = a[0] + a[1]*x + a[2]*y + a[3]*x^2 + a[4]*x*y + a[5]*y^2 + a[6]*x^3 + a[7]*x^2*y + a[8]*x*y^2 + a[9]*y^3
    class LightPollutionGradientPolynomial3N : public LightPollutionGradientBase {
        public:
            LightPollutionGradientPolynomial3N(int width, int height) : LightPollutionGradientBase(width, height) {
                initialize_parameters(10); // Initialize with 10 parameters
            };

            LightPollutionGradientPolynomial3N(const LightPollutionGradientPolynomial3N &other) : LightPollutionGradientBase(other) {};

            virtual std::unique_ptr<LightPollutionGradientBase> clone() const override {
                auto clone = std::make_unique<LightPollutionGradientPolynomial3N>(*this);
                return clone;
            };

            double get_value(double x, double y) const override {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return a[0] + a[1]*x + a[2]*y + a[3]*x*x + a[4]*x*y + a[5]*y*y + a[6]*x*x*x + a[7]*x*x*y + a[8]*x*y*y + a[9]*y*y*y;
            };

            std::vector<double> get_derivative(double x, double y) const override   {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return {1.0, x, y, x*x, x*y, y*y, x*x*x, x*x*y, x*y*y, y*y*y};
            };

            void set_parameters(const std::vector<double>& params) override {
                if (params.size() != 10) {
                    throw std::invalid_argument("Expected 10 parameters for LightPollutionGradientPolynomial3N");
                }
                m_parameters = params;
            };
    };


    // y = a[0] + a[1]*x + a[2]*y + a[3]*x^2 + a[4]*x*y + a[5]*y^2 + a[6]*x^3 + a[7]*x^2*y + a[8]*x*y^2 + a[9]*y^3 + a[10]*x^4 + a[11]*x^3*y + a[12]*x^2*y^2 + a[13]*x*y^3 + a[14]*y^4
    class LightPollutionGradientPolynomial4N : public LightPollutionGradientBase {
        public:
            LightPollutionGradientPolynomial4N(int width, int height) : LightPollutionGradientBase(width, height) {
                initialize_parameters(15); // Initialize with 15 parameters
            };

            LightPollutionGradientPolynomial4N(const LightPollutionGradientPolynomial4N &other) = default;

            virtual std::unique_ptr<LightPollutionGradientBase> clone() const override {
                auto clone = std::make_unique<LightPollutionGradientPolynomial4N>(*this);
                return clone;
            };

            double get_value(double x, double y) const override {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return a[0] + a[1]*x + a[2]*y + a[3]*x*x + a[4]*x*y + a[5]*y*y + a[6]*x*x*x + a[7]*x*x*y + a[8]*x*y*y + a[9]*y*y*y + a[10]*x*x*x*x + a[11]*x*x*x*y + a[12]*x*x*y*y + a[13]*x*y*y*y + a[14]*y*y*y*y;
            };

            std::vector<double> get_derivative(double x, double y) const override   {
                normalize_coordinates(&x, &y);
                const std::vector<double> a = get_parameters();
                return {1.0, x, y, x*x, x*y, y*y, x*x*x, x*x*y, x*y*y, y*y*y, x*x*x*x, x*x*x*y, x*x*y*y, x*y*y*y, y*y*y*y};
            };

            void set_parameters(const std::vector<double>& params) override {
                if (params.size() != 15) {
                    throw std::invalid_argument("Expected 15 parameters for LightPollutionGradientPolynomial4N");
                }
                m_parameters = params;
            };
    };


    class GradientFunctionsFactory {
        public:
            static std::unique_ptr<LightPollutionGradientBase> get_gradient_function(const std::string &function_type, int width, int height)   {
                auto it = s_factory_map.find(function_type);
                if (it == s_factory_map.end()) {
                    throw std::invalid_argument("Unsupported gradient function type: " + function_type);
                }
                return it->second(width, height);
            };

            static std::vector<std::string> get_supported_function_types() {
                std::vector<std::string> function_types;
                for (const auto &entry : s_factory_map) {
                    function_types.push_back(entry.first);
                }
                return function_types;
            };

        private:
            static std::map<std::string, std::function<std::unique_ptr<LightPollutionGradientBase>(int, int)>> s_factory_map;

    };
}
