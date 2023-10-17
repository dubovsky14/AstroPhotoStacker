#include "../headers/ImageStretching.h"

#include <stdexcept>


using namespace AstroPhotoStacker;
using namespace std;

double ImageStretcher::get_scale_factor(double pixel_value, StretchingType stretching_type)    {
    if (pixel_value == 0)   return 0;
    double result;
    if (stretching_type == StretchingType::logarithmic) {
        result = std::log10(1 + pixel_value) / std::log10(1 + m_max_value);
    } else if (stretching_type == StretchingType::quadratic) {
        result = std::pow(pixel_value, 2) / std::pow(m_max_value, 2);
    } else if (stretching_type == StretchingType::linear) {
        result = pixel_value / m_max_value;
    }
    else {
        throw std::runtime_error("Unknown stretching type");
    }
    result *= m_max_value/pixel_value;
    return result;
};

double ImageStretcher::get_rgb_brightness(unsigned int i_pixel) {
    double rgb_brightness = 0;
    for (unsigned int i_color = 0; i_color < m_image->size(); i_color++) {
        rgb_brightness += m_image->at(i_color)[i_pixel];
    }
    rgb_brightness /= m_image->size();
    return rgb_brightness;
};

void ImageStretcher::stretch_image(StretchingType stretching_type) {
    for (unsigned int i_pixel = 0; i_pixel < m_image->at(0).size(); i_pixel++)  {
        // calculate overall brightness of the pixels from all (usually 3) channels
        double rgb_sum = 0;
        for (const std::vector<double> &color : *m_image) {
            rgb_sum += color[i_pixel];
        }
        rgb_sum /= m_image->size();

        // get stretching scale factor and use it to adjust pixel values
        const double scale_factor = get_scale_factor(rgb_sum, stretching_type);
        for (std::vector<double> &color : *m_image) {
            color[i_pixel] = min(scale_factor*color[i_pixel], m_max_value);
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


void ImageStretcher::initialize_linear_to_logarithmic_variables()   {
    m_integrated_histogram = get_integrated_histogram_from_rgb_image(*m_image, m_max_value);

    const double transition_fraction = 0.4; // this fraction of pixels will not be stretched logarithmically
    for (unsigned int i = 0; i < m_integrated_histogram.size(); i++) {
        if (m_integrated_histogram[i] > transition_fraction ) {
            m_linear_to_logarithmic_transition_point_x = i;
            m_linear_to_logarithmic_transition_point_y = i;
            break;
        }
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
        double rgb_sum = 0.;
        for (const vector<double> &color : image)   {
            rgb_sum += color[i_pixel];
        }
        rgb_sum /= image.size();
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