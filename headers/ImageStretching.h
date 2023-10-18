#pragma once

#include <cmath>
#include <vector>


namespace AstroPhotoStacker {
    enum class StretchingType{logarithmic, quadratic, linear, lin_log_sigmoid};

    class ImageStretcher {
        public:
            ImageStretcher(std::vector<std::vector<double>> *image) : m_image(image) {};

            void set_max_value(double max_value)            {m_max_value = max_value;};

            void stretch_image(StretchingType stretching_type);

            void apply_black_point(double black_pixels_fraction);

        private:
            double get_scale_factor(double pixel_value, StretchingType stretching_type);
            double get_rgb_brightness(unsigned int i_pixel);

            std::vector<std::vector<double>> *m_image   = nullptr;
            double m_max_value                          = 1 << 14;

            void initialize_ling_log_sigmoid_variables();
            std::vector<double> m_integrated_histogram;
            unsigned int    m_linear_to_logarithmic_transition_point_x = 0;
            double          m_linear_to_logarithmic_transition_point_y = 0;


    };

    template <typename input_array_type>
    std::vector<unsigned int> get_histogram_from_monochrome_image(const input_array_type *input_array, unsigned int array_size, int output_size)  {
        std::vector<unsigned int> result(output_size, 0);
        for (unsigned int i_pixel = 0; i_pixel < array_size; i_pixel++) {
            result[int(input_array[i_pixel])] += 1;
        }
        return result;
    }

    std::vector<unsigned int> get_histogram_from_rgb_image(const std::vector<std::vector<double>> &image, int output_size);

    // fraction of events with value less than or equal to the given value
    std::vector<double> get_integrated_histogram_from_rgb_image(const std::vector<std::vector<double>> &image, int output_size);

    unsigned int get_brightness_from_fraction(const std::vector<std::vector<double>> &image, double fraction);

    float sigmoid(float x, float center, float width);

}

