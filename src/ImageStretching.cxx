#include "../headers/ImageStretching.h"

#include <stdexcept>


using namespace AstroPhotoStacker;
using namespace std;

double AstroPhotoStacker::get_scale_factor(double pixel_value, double max_value, StretchingType stretching_type)    {
    if (pixel_value == 0)   return 0;
    double result;
    if (stretching_type == StretchingType::logarithmic) {
        result = std::log10(1 + pixel_value) / std::log10(1 + max_value);
    } else if (stretching_type == StretchingType::quadratic) {
        result = std::pow(pixel_value, 2) / std::pow(max_value, 2);
    } else if (stretching_type == StretchingType::linear) {
        result = pixel_value / max_value;
    }
    else {
        throw std::runtime_error("Unknown stretching type");
    }
    result *= max_value/pixel_value;
    return result;
};

void AstroPhotoStacker::stretch_image(std::vector<std::vector<double> > *image, double max_value, StretchingType stretching_type, bool keep_rgb_ratio) {
    if (!keep_rgb_ratio)    {
        for (std::vector<double> &color : *image) {
            for (double &pixel : color) {
                pixel *= get_scale_factor(pixel, max_value, stretching_type);
            }
        }
    }
    else    {
        for (unsigned int i_pixel = 0; i_pixel < image->at(0).size(); i_pixel++)  {
            // calculate overall brightness of the pixels from all (usually 3) channels
            double rgb_sum = 0;
            for (const std::vector<double> &color : *image) {
                rgb_sum += color[i_pixel];
            }
            rgb_sum /= image->size();

            // get stretching scale factor and use it to adjust pixel values
            const double scale_factor = get_scale_factor(rgb_sum, max_value, stretching_type);
            for (std::vector<double> &color : *image) {
                color[i_pixel] = min(scale_factor*color[i_pixel], max_value);
            }
        }
    }
}


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
