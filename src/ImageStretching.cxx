#include "../headers/ImageStretching.h"

#include <stdexcept>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

double ImageStretcher::get_scale_factor(double pixel_value, StretchingType stretching_type)    {
    if (pixel_value == 0)   return 0;
    if (stretching_type == StretchingType::logarithmic) {
        return (m_max_value/pixel_value)*std::log10(1 + pixel_value) / std::log10(1 + m_max_value);
    } else if (stretching_type == StretchingType::quadratic) {
        return (m_max_value/pixel_value)*std::pow(pixel_value, 2) / std::pow(m_max_value, 2);
    } else if (stretching_type == StretchingType::linear) {
        return 1.;
    } else if (stretching_type == StretchingType::sqrt) {
        return (m_max_value/pixel_value)*std::sqrt(pixel_value) / std::sqrt(m_max_value);
    }
    else if (stretching_type == StretchingType::sqrt_and_lin) {
        const double sqrt_sf = get_scale_factor(pixel_value, StretchingType::sqrt);
        const double lin_sf = get_scale_factor(pixel_value, StretchingType::linear);
        const double wight_linear = pixel_value / m_max_value;
        return wight_linear*lin_sf + (1 - wight_linear)*sqrt_sf;
    }
    else if (stretching_type == StretchingType::lin_log_sigmoid) {
        const double sigmoid_value = sigmoid(pixel_value, m_linear_to_logarithmic_transition_point_x, m_linear_to_logarithmic_transition_point_x*0.3);
        const double log_scale_factor = get_scale_factor(pixel_value, StretchingType::logarithmic);
        return (1 - sigmoid_value) + sigmoid_value*log_scale_factor;
    }
    else if (stretching_type == StretchingType::uniform) {
        return m_uniform_stretching_scale_factors[pixel_value]/pixel_value;
    }


    throw std::runtime_error("Unknown stretching type");

};

double ImageStretcher::get_rgb_brightness(unsigned int i_pixel) {
    return AstroPhotoStacker::get_rgb_brightness(i_pixel, *m_image);
};

double AstroPhotoStacker::get_rgb_brightness(unsigned int i_pixel, const std::vector<std::vector<double>> &image) {
    double rgb_brightness = 0;
    for (unsigned int i_color = 0; i_color < image.size(); i_color++) {
        if (i_color != 1)   {
            rgb_brightness += image.at(i_color)[i_pixel];
        }
        else {
            rgb_brightness += 0.5*image.at(i_color)[i_pixel];
        }
    }
    rgb_brightness /= image.size();
    return rgb_brightness;
};


void ImageStretcher::stretch_image(StretchingType stretching_type) {
    if (stretching_type == StretchingType::lin_log_sigmoid) {
        initialize_ling_log_sigmoid_variables();
    }
    if (stretching_type == StretchingType::uniform) {
        initialize_uniform_stretching_variables();
    }

    for (unsigned int i_pixel = 0; i_pixel < m_image->at(0).size(); i_pixel++)  {
        // calculate overall brightness of the pixels from all (usually 3) channels
        double rgb_sum = get_rgb_brightness(i_pixel);

        // get stretching scale factor and use it to adjust pixel values
        const double scale_factor = get_scale_factor(rgb_sum, stretching_type);
        for (unsigned int i_color = 0; i_color < m_image->size(); i_color++) {
            vector<double> &color = m_image->at(i_color);
            color[i_pixel] = min((i_color == 1 ? 2. : 1)*scale_factor*color[i_pixel], m_max_value);
        }
    }

    // check if uniform stretching works
    if (stretching_type == StretchingType::uniform) {
        vector<float> thresholds = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        vector<float> brightnesses(thresholds.size(), 0);
        for (unsigned int i = 0; i < thresholds.size(); i++) {
            brightnesses[i] = get_brightness_from_fraction(*m_image, thresholds[i]);
        }
        for (unsigned int i = 0; i < thresholds.size(); i++) {
            cout << "Threshold: " << thresholds[i] << " brightness: " << brightnesses[i] << "\n";
        }
    }
}

void ImageStretcher::apply_black_point(double black_pixels_fraction)  {
    const double black_point = get_brightness_from_fraction(*m_image, black_pixels_fraction);
    for (unsigned int i_pixel = 0; i_pixel < m_image->at(0).size(); i_pixel++) {
        const double rgb_brightness = get_rgb_brightness(i_pixel);
        const double scale_factor = (rgb_brightness < black_point) ? 0 : (rgb_brightness - black_point)/(m_max_value - black_point);

        for (unsigned int i_color = 0; i_color < m_image->size(); i_color++) {
            double &pixel_value = m_image->at(i_color)[i_pixel];
            pixel_value = min(scale_factor*pixel_value, m_max_value);
        }
    }
};


void ImageStretcher::initialize_ling_log_sigmoid_variables()   {
    m_integrated_histogram = get_integrated_histogram_from_rgb_image(*m_image, m_max_value);

    const double transition_fraction = 0.3; // this fraction of pixels will not be stretched logarithmically
    for (unsigned int i = 0; i < m_integrated_histogram.size(); i++) {
        if (m_integrated_histogram[i] > transition_fraction ) {
            m_linear_to_logarithmic_transition_point_x = 0.9*i;
            m_linear_to_logarithmic_transition_point_y = 0.9*i;
            break;
        }
    }
};


void ImageStretcher::initialize_uniform_stretching_variables()  {
    m_integrated_histogram = get_integrated_histogram_from_rgb_image(*m_image, m_max_value);
    m_uniform_stretching_scale_factors.resize(m_integrated_histogram.size(), 0);
    for (unsigned int i = 0; i < m_integrated_histogram.size(); i++) {
        m_uniform_stretching_scale_factors[i] = 0.25*m_max_value*m_integrated_histogram[i];
    }
};

vector<unsigned int> AstroPhotoStacker::get_histogram_from_rgb_image(const vector<vector<double>> &image, int output_size)  {
    if (image.size() == 0)  {
        throw runtime_error("Cannot compute histogram of an empty image");
    }
    const unsigned int n_pixels = image[0].size();
    for (const vector<double> &color : image)   {
        if (color.size() != n_pixels)   {
            throw runtime_error("Inconsistent image! Different number of pixels across the channels");
        }
    }

    vector<unsigned int> result(output_size, 0);
    for (unsigned int i_pixel = 0; i_pixel < n_pixels; i_pixel++)   {
        double rgb_sum = get_rgb_brightness(i_pixel, image);
        result[(unsigned int)(rgb_sum)] += 1;
    }
    return result;
};


std::vector<double> AstroPhotoStacker::get_integrated_histogram_from_rgb_image(const std::vector<std::vector<double>> &image, int output_size)   {
    const vector<unsigned int> histogram = get_histogram_from_rgb_image(image, output_size);
    vector<double> result(output_size, 0);

    result[0] = histogram[0];
    for (int i = 1; i < output_size; i++)   {
        result[i] = result[i-1] + histogram[i];
    }

    for (double &value : result)    {
        value /= result.back();
    }

    return result;
};


unsigned int AstroPhotoStacker::get_brightness_from_fraction(const std::vector<std::vector<double>> &image, double fraction)  {
    const vector<double> integrated_histogram = get_integrated_histogram_from_rgb_image(image, 65535);
    for (unsigned int i = 0; i < integrated_histogram.size(); i++) {
        if (integrated_histogram[i] > fraction) {
            return i;
        }
    }
    return 65535;
};

float AstroPhotoStacker::sigmoid(float x, float center, float width)   {
    return 1. / (1. + exp(-(x - center)/width));
};
